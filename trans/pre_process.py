import os
import sys
import json
import subprocess
import shutil
import tempfile
import base64
import time
import re
import math
import stat
from typing import Dict, List, Tuple, Set, Any, Union
from copy import deepcopy
from collections import defaultdict, deque
from functools import reduce
from datetime import datetime, timedelta
from pathlib import Path
import networkx as nx
import matplotlib.pyplot as plt
import tiktoken
import chardet
import openai
from openai import OpenAI, AzureOpenAI
from pydantic import BaseModel
import anthropic
from anthropic import InternalServerError
import replicate
from concurrent.futures import ThreadPoolExecutor
import concurrent.futures
from pycparser import c_parser, c_ast
import clang.cindex
from clang.cindex import (
    Index,
    CursorKind,
    TokenKind,
    CompilationDatabase,
    CompilationDatabaseError
)
from collections import defaultdict

# from google.generativeai.protos import Content, Part
# import google.generativeai as genai
# from openai import RateLimitError, APIError
# from testGen.main import print_hello

clang.cindex.Config.set_library_file('/usr/lib/llvm-19/lib/libclang.so.1')
# clang.cindex.Config.set_library_file('/opt/homebrew/opt/llvm/lib/libclang.dylib')  # for macOS


from utils_api import (
    # normal
    read_json,
    write_json,
    read_file,
    write_file,
    copy_file,
    delete_file,
    create_file,
    append_file,
    create_permissioned_file,
    rename_directory,
    create_directory,
    delete_directory,
    copy_directory,
    clone_directory,
    create_backup_directory,
    recreate_directory,
    run_script,
    run_script_wo_log,
    find_compile_commands_json,
    deduplicate_compile_commands,
    count_file_lines,
    get_timestamp,
    get_coverage,
    get_last_directory,
    grant_permissions,
    execute_command,
    get_list_path,

    # translation
    set_log,
    create_path_config,
    extract_all_paths,
    obtain_metadata,
    normalize_translation_metadata,
    denormalize_translation_metadata,
    normalize_metafiles,
    denormalize_metafiles,
    # denormalize_is_program_path,
    # normalize_is_program_path,
    normalize_dep_data,
    denormalize_block_path,
    read_specific_lines,
    get_llm_flag,
    get_name_key,
    parse_def_loc,
)

from c_parser_api import (
    analyze_dependencies,
    #analyze_function,
    analyze_call_relationship,
    p_f,
    #parse_files_c,
    get_files_list,
    #analyze_macros_llm,
    detect_include_guards,
    delete_guards,
    delete_macro_defs,
    delete_independent_defs,
    delete_global_defs,
    generate_metadata,
    generate_macro_metadata,
    define_blocks,
    parse_all,
    detect_independent_macros,
    find_headers,
    get_build_path,
    #detect_cfg,
    get_compile_json,
    #insert_expanded_code,
    is_system_file,
    setup_compile_json,
)


from llm_api import (
    LLMInterface,
    init_prompt_count, 
    #set_exp_data,
    occupy_llm,
    configure_llm,
    shutdown_llm,
    save_coverage_report,
    get_dir_struct,
    get_claude_model,
)


MACRO_HOME = "/root/SmartC2Rust/macro"
TRANS_HOME = "/root/SmartC2Rust/trans"
C_PARSER_HOME = "/root/kiso-parser-c"


DEBUG_LLM = False
IF_MODIFY_FILE = False # Handling of if_condition # Disable this at the initial stage, as the design is not yet finalized

REPAIR_MAX = 500
guided_line = 20 #10

removed_statement = '/* momotaro removed */'

class Item(BaseModel):
    category: str
    element: str
    start_line: int
    end_line: int
    block_type: str
    c_flow: Dict[str, Any]


##############################################
##### Translation helper functions
##############################################

def get_exp_path(file_path, average, exp_dir, target):
    file_path = file_path.replace('.', '_')
    file_path = file_path.replace('/', '_')
    experiment_path = exp_dir + "/" + target + "_" + f"{average}" + "_" + file_path + ".json"

    return experiment_path


def set_exp_data(file_path, average, exp_dir, target, log_file_path, trial_id, moment_path):
    exp_data = {}
    experiment_path = get_exp_path(file_path, average, exp_dir, target)
    exp_data['experiment_path'] = experiment_path
    if os.path.exists(experiment_path):
        delete_file(experiment_path)

    exp_data['file_path'] = exp_dir + "/" + file_path
    exp_data['average'] = average
    exp_data['log_file_path'] = log_file_path
    exp_data['repair_count'] = 0

    moment_json = read_json(moment_path)
    moment_json[target][trial_id]['paths'].append(file_path)
    moment_json[target][trial_id]['exp_logs'].append(experiment_path)
    write_json(moment_path, moment_json)
    
    return exp_data



##############################################
##### common functions
##############################################

def remove_base_path(full_path, base_path):
    base_path = os.path.normpath(base_path) # Normalize the base path
    full_path = os.path.normpath(full_path) # Normalize the full path
    
    base_length = len(base_path) # Get the length of the base path
    
    if full_path.startswith(base_path): # If the full path starts with the base path, remove that part
        return full_path[base_length:].strip(os.path.sep)
    else:
        return full_path

# new one
def is_commented_out(line, in_multiline_comment):
    if '/*' in line:
        in_multiline_comment = True
    if '*/' in line:
        in_multiline_comment = False
        return True, in_multiline_comment
    if in_multiline_comment or line.strip().startswith('//'):
        return True, in_multiline_comment
    return False, in_multiline_comment


##############################################
##### C: parse files
##############################################


def dfs(file, file_dependencies, visited, current_group):
    stack = [file]
    while stack:
        current_file = stack.pop()
        if current_file not in visited:
            visited.add(current_file)
            current_group.add(current_file)
            for dep in file_dependencies.get(current_file, []):
                if dep not in visited:
                    stack.append(dep)
    return current_group

def replace_includes(dep_json_file): # Fix include statements for files outside the same module
    dep_data = read_json(dep_json_file)

    included_file_dict = {}
    source_file_dict = {}
    for entry in dep_data:
        source_file = entry['source']
        module_name = entry['module']
        included_files = entry['included']
        
        if module_name not in source_file_dict:
            source_file_dict[module_name] = []
        source_file_dict[module_name].append(source_file)

        if module_name not in included_file_dict:
            included_file_dict[module_name] = []
        included_file_dict[module_name].extend(included_files)

    
    for module_name, files_list in included_file_dict.items():
        for included_file in files_list:
            if included_file not in source_file_dict[module_name]: # Exclude source files within the same module
                for source_file in source_file_dict[module_name]:
                    # Open each included file and replace #include directives
                    if os.path.exists(included_file):
                        with open(included_file, 'r') as file:
                            lines = file.readlines()

                        updated_lines = []
                        for line in lines:
                            source_file_basename = os.path.basename(source_file)
                            if re.match(r'#\s*include', line.strip()) and source_file_basename in line: #source_file in line:
                            #if line.strip().startswith('#include') and source_file in line: # It might be better to track line numbers here
                                #if not same_module:
                                module_name_basename = os.path.basename(module_name)
                                new_include = line.replace(source_file_basename, module_name_basename) #module_name)
                                #updated_lines.append('test')
                                updated_lines.append(new_include)
                                #else:
                                #    updated_lines.append('//heder removed') # Even if changed to //heder removed, later processing will remove comments
                            else:
                                updated_lines.append(line)

                        # Overwrite the file
                        with open(included_file, 'w') as file:
                            file.writelines(updated_lines)
                    else:
                        print(f"File not found: {included_file}")


def detect_file_cycle(dep_json_path):
    copy_file(dep_json_path, "tmp_dep.json")

    # consolidate modules
    consolidate_to_module(dep_json_path)

    # Detect
    dep_json = read_json(dep_json_path)
    dependencies = {}
    
    # Build graph and create file list
    for dep in dep_json:
        source = dep['source']
        dependencies[source] = dep['indirect_include']
    
    sorted_files = topological_flow_sort(dependencies)
    #sorted_files, cycles = topological_file_sort(dependencies)

    cycles = find_minimal_circular_dependencies(dep_json) #find_circular_dependencies(dependencies)
    print("Circular dependencies found:")
    for cycle in cycles:
        print(" -> ".join(cycle + [cycle[0]]))

    dependency_sets = find_connected_dependency_sets(cycles)
    # Display results
    print("Connected dependency sets:")
    for i, dep_set in enumerate(dependency_sets, 1):
        print(f"\nSet {i}:")
        print(", ".join(sorted(dep_set)))

    
    if len(dependency_sets) > 0:
        print("dependency_sets")

    copy_file("tmp_dep.json", dep_json_path)
    delete_file("tmp_dep.json")

##################################################

def delete_include_guards(all_macros_path, guards):
    macros = read_json(all_macros_path)
    for item in guards:
        if item['name'] in macros:
            # Collect indices to remove from the end (to delete from the back)
            indices_to_remove = []
            for i, entry in enumerate(macros[item['name']]):
                if entry['file_path'] == item['file_path']:
                    indices_to_remove.append(i)
            
            # Remove elements from the end
            for index in sorted(indices_to_remove, reverse=True):
                macros[item['name']].pop(index)
            
            # If the array becomes empty, delete the key itself
            if not macros[item['name']]:
                del macros[item['name']]

    write_json(all_macros_path, macros)



##############################################
##### categorize blocks
##############################################

def write_func_flag(all_macros_path):
    macro_data = read_json(all_macros_path)

    for macro, items in macro_data.items():
        for item in items:
            print(f"Searching flag, {macro} for {item['file_path']}")
            if item['category'] == 'macro_var' and item['func_flag'] == True:
            #if 'category' in item and item['category'] == 'macro_var' and 'func_flag' in item and item['func_flag'] == True:
                item['category'] = 'macro_func'

    write_json(all_macros_path, macro_data)


def convert_dict_to_frozenset(d):
    """
    Convert a dictionary to a frozenset of key-value pairs, with lists converted to tuples and nested dicts handled recursively.
    """
    converted_items = []
    for key, value in d.items():
        if isinstance(value, list):
            value = tuple(value)
        elif isinstance(value, dict):
            value = convert_dict_to_frozenset(value)  # Recursively convert dictionaries
        converted_items.append((key, value))
    return frozenset(converted_items)


def is_include_guard(item, meta_data):
    # Minimum requirement: check category and #if directive
    if not (item['category'] == 'conditional'): # and item['directive'] == '#if'
        return False
    
    # Check whether it is the outermost conditional compilation block in the file
    # (minimum start_line and maximum end_line)
    is_outermost = False
    item_start = item['start_line']
    item_end = item['end_line']
    
    total_count = 0
    count = 0
    for other in meta_data:
        if (other != item):
            other_start = other['start_line']
            other_end = other['end_line']
            
            # If the current block fully contains all other blocks,
            # it is considered an include guard
            if ( other_start >= item_start  and 
                item_end >= other_end):
                count += 1

            total_count += 1
    
    if total_count == count:
        is_outermost = True
    return is_outermost


# conditional may also be nested inside
"""
struct test {
#ifdef

#else

#endif

}
"""

def categorize_blocks(file_path, raw_dir, meta_dir, all_macros_path):
    
    print("------------- Generate blocks metadata -------------")
    meta_data, meta_path = obtain_metadata(file_path, meta_dir, False, None, "def")

    #print(meta_path)
    if meta_data is None: # added
        return

    for item in meta_data:
        # Determine whether it is an include guard # However, this should no longer exist
        item['include_guard'] = is_include_guard(item, meta_data)

    for item in meta_data:
        # if item['category'] == "global_var": # This conditional branch seems incorrect
        #    continue

        p_start_line = item['start_line']
        p_end_line = item['end_line']

        if item['include_guard'] is True:
            continue
        
        for other_item in meta_data:
            o_start_line = other_item['start_line']
            o_end_line = other_item['end_line']
            
            # Conditions where item contains other_item:
            # 1. other_item is not itself
            # 2. item's start line is before other's start line
            # 3. item's end line is after other's end line
            if (item != other_item and 
                p_start_line <= o_start_line and 
                p_end_line >= o_end_line):
                
                if 'components' not in item:
                    item['components'] = []
                
                component_copy = other_item.copy()
                item['components'].append(component_copy)
                other_item['child'] = True
    
    write_json(meta_path, meta_data)

    meta_data = read_json(meta_path)

    # Add only items without the child flag to the new metadata
    new_meta_data = []
    for item in meta_data:
        if 'child' not in item:
            new_meta_data.append(item)
    
    write_json(meta_path, new_meta_data)


    meta_data = read_json(meta_path)
    for item in meta_data:
        if item['category'] == 'function': #for item in meta_data['function']:
            item['block_type'] = 'function'

        elif item['category'] in ['macro_func', 'macro_var', 'data_type', 'global_var', 'header_include']:
            item['block_type'] = 'others'

        else:
            item['block_type'] = 'conditional'

    write_json(meta_path, meta_data)


def insert_c_code(file_path, raw_dir, meta_dir):
    meta_data, meta_path = obtain_metadata(file_path, meta_dir, False, None, "def")

    if meta_data is None:
        return
    
    for item in meta_data:
        item['c_code'] = read_specific_lines(item['file_path'], item['start_line'], item['end_line'])
    
    write_json(meta_path, meta_data)


def find_element_id(name, file_path):
    element_id = None
    meta_data, meta_path = obtain_metadata(file_path, meta_dir, False, None, "def")
    if meta_data is None:
        return None
    for item in meta_data:
        if item['name'] == name:
            element_id = item['element_id']
            break
    return element_id



def get_both_dep(file_path, dep_json_path):
    sum_include_files = []
    include_files = get_indirect_includes(file_path, dep_json_path)
    #if include_files is not None:
    sum_include_files.extend(include_files)
    #print(include_files)
    included_files = get_indirect_includeds(file_path, dep_json_path)
    #if included_files is not None:
    sum_include_files.extend(included_files)
    #print(included_files)

    sum_include_files.append(file_path)

    #if len(sum_include_files) > 1:
    sum_include_files = list(set(sum_include_files))
    
    return sum_include_files


##############################################
##### Modify lines by the block order
##############################################

# Revised version of the function with an added post-process step to more strictly control the ordering of nodes within each strongly connected component after detection
def topological_flow_sort(dependencies):
    def dfs(node, stack, on_stack, visited, component):
        index[node] = len(index)
        low_link[node] = index[node]
        stack.append(node)
        on_stack[node] = True
        visited.add(node)
        
        for neighbor in dependencies.get(node, []):
            if neighbor not in visited:
                dfs(neighbor, stack, on_stack, visited, component)
                low_link[node] = min(low_link[node], low_link[neighbor])
            elif on_stack[neighbor]:
                low_link[node] = min(low_link[node], index[neighbor])
        
        if low_link[node] == index[node]:
            scc = []
            while True:
                w = stack.pop()
                on_stack[w] = False
                scc.append(w)
                if w == node:
                    break
            component.append(scc)

    index = {}
    low_link = {}
    on_stack = defaultdict(bool)
    visited = set()
    stack = []
    component = []

    # Run DFS for each node to find strongly connected components
    for node in sorted(dependencies):
    # for node in dependencies:
        if node not in visited:
            dfs(node, stack, on_stack, visited, component)

    # Expand nodes in each strongly connected component into a list to obtain topological order
    topo_sort = []
    for scc in component:
        # Process to control ordering within strongly connected components
        if len(scc) > 1:
            # If a cycle is detected, reorder nodes appropriately
            scc_sorted = sorted(scc, key=lambda x: index[x])
            topo_sort.extend(scc_sorted)
        else:
            topo_sort.extend(scc)

    return topo_sort


def find_circular_dependencies(function_calls):
    # Build dependency graph of functions
    graph = defaultdict(list)
    for function, dependencies in function_calls.items():
        for dep in dependencies:
            graph[function].append(dep)
    
    # Use Tarjan's algorithm to find strongly connected components (SCC)
    def tarjan(node, low_link, disc_time, stack, on_stack, scc):
        nonlocal time
        disc_time[node] = time
        low_link[node] = time
        time += 1
        stack.append(node)
        on_stack[node] = True
        
        for neighbor in graph[node]:
            if neighbor not in disc_time:
                tarjan(neighbor, low_link, disc_time, stack, on_stack, scc)
                low_link[node] = min(low_link[node], low_link[neighbor])
            elif on_stack[neighbor]:
                low_link[node] = min(low_link[node], disc_time[neighbor])
        
        if low_link[node] == disc_time[node]:
            component = []
            while True:
                v = stack.pop()
                on_stack[v] = False
                component.append(v)
                if v == node:
                    break
            if len(component) > 1 or (len(component) == 1 and component[0] in graph[component[0]]):
                scc.append(component)
    
    # Execute Tarjan's algorithm
    disc_time = {}
    low_link = {}
    on_stack = {}
    stack = []
    scc = []
    time = 0
    
    for node in function_calls.keys():
        if node not in disc_time:
            tarjan(node, low_link, disc_time, stack, on_stack, scc)
    
    # Output results
    if scc:
        print("Detected Circular Dependency Groups:")
        for i, group in enumerate(scc, 1):
            print(f"Group {i}: {group}")
    
    print(scc)
    return scc


def find_parent_def_key(cashed, meta_dir, use_item, use_file_path, use_start_line, program_files):

    use_parent_key = None
    #parent_name = None
    name = None
    parent_block_start = None

    if use_file_path not in cashed:
        use_meta_data, use_meta_path = obtain_metadata(use_file_path, meta_dir, False, None, "def")
        #use_meta_data, use_meta_path = obtain_metadata(use_file_path, meta_dir, False, None, "def")
        cashed[use_file_path] = use_meta_data
    else:
        use_meta_path = obtain_metadata(use_file_path, meta_dir, False, True, "def")
        use_meta_data = cashed[use_file_path]
    
    found = False
    for key, item in use_meta_data.items():
        #print(item)
        ## This should actually be checked: why are some becoming undefined? Probably an issue with the parser.
        if 'file_path' in item: # This must be checked first, otherwise some definitions become undefined. Why???
            def_file_path = item['file_path'] # In the case of IFDEF/IF directive.
        
        elif 'definition' in item:
            definition = item['definition']
            def_file_path, def_start, def_column = parse_def_loc(definition)
        
        else:
            raise ValueError("Must find the def filename.")

        if is_system_file(def_file_path, program_files):
            continue

        block_start = item['block_start']
        block_end = item['block_end']

        if block_start <= use_start_line <= block_end:
            if 'type' in item:
                if item['type'] in ["IFDEF", "IFNDEF", "IF", "ELIF"]: # IF
                    name = item['type']
                elif 'name' in item:
                    name = item['name']
                else:
                    raise ValueError("The name should be defined by 'name' or 'type'.")
            else:
                name = item["name"]
            """
            if 'name' not in item:
                #print(item)
                item['name'] = "IF"
            parent_name = item['name']
            """
            parent_block_start = block_start
            use_parent_key = f"{name}:{def_file_path}:{item['block_start']}:{item['block_end']}"
            found = True
            break

    if found is False:
        raise ValueError(f"Shoud find for {use_item} at {use_meta_path}")

    return cashed, use_parent_key, name, parent_block_start


#def build_c_graph(meta_dir, target_dir, is_program_path, block_path):
def define_block_order(meta_dir, div_meta_dir, target_dir, database_dir, is_program_path, block_path):

    cashed = {}
    div_cashed = {}
    #order = set(read_json(is_program_path))
    program_files = set(read_json(is_program_path))

    dependencies = defaultdict(list)
    for file_path in program_files:
        if file_path not in cashed:
            meta_data, meta_path = obtain_metadata(file_path, meta_dir, False, None, "def")
            cashed[file_path] = meta_data
        else:
            meta_data = cashed[file_path]
        
        # Handle the case where meta_data is in dictionary format
        # Sort by start_line (convert dictionary values to a list and sort)
        #if isinstance(meta_data, dict):
        #sorted_items = sorted(meta_data.items(), key=lambda x: x[1]['start_line'])
        sorted_items = sorted(
            meta_data.items(),
            key=lambda item: item[1]['block_start']
        )
        """
        else:
            # Backward compatibility for the old format (list)
            #sorted_items = [(None, item) for item in sorted(meta_data, key=lambda x: x['start_line'])]
            sorted_items = [(None, item) for item in sorted(meta_data, key=lambda x: x['block_start'])]
        """

        # Build dependencies
        for key, item in sorted_items:
            # Skip declarations only (no implementation body)
            if item.get('kind', '').endswith('_decl'):
                continue
                
            """
            if item['category'] != sort_type: 
            #if item['block_type'] != sort_type: #"function":
                continue
            """
            if 'type' in item:
                if item['type'] in ["IFDEF", "IFNDEF", "IF", "ELIF"]: # IF
                    name = item['type']
                elif 'name' in item:
                    name = item['name']
                else:
                    raise ValueError("The name should be defined by 'name' or 'type'.")
            else:
                name = item["name"]

            def_file_path = file_path #item["file_path"]  # Use the current file_path if file_path is not present
            #start_line = item["start_line"]
            #end_line = item["end_line"]
            start_line = item["block_start"]
            end_line = item["block_end"]
            
            """
            if key is not None:
                key_name = f"{key}:{end_line}"
            """
            key_name = f"{name}:{def_file_path}:{start_line}:{end_line}"

            if "uses" not in item:
                item["uses"] = []
            uses_items = item["uses"]                        
            
            uses_list = []
            for use_item in uses_items:
                #print(use_item)
                use_name = use_item['name']
                use_definition = use_item['definition']
                use_file_path = use_item['file_path']
                use_start_line = use_item['start_line']

                if is_system_file(use_file_path, program_files):
                    continue

                """
                if use_file_path not in div_cashed:
                    use_meta_data, use_meta_path = obtain_metadata(use_file_path,div_meta_dir, False, None, "def")
                    #use_meta_data, use_meta_path = obtain_metadata(use_file_path, meta_dir, False, None, "def")
                    div_cashed[use_file_path] = use_meta_data
                else:
                    use_meta_data = div_cashed[use_file_path]
                
                if use_meta_data is None:
                    #print(use_meta_path)
                    continue

                use_def_key = f"{use_name}:{use_file_path}:{use_def_start}"
                if use_def_key not in use_meta_data:
                    #continue
                    raise ValueError("Should find the corresponding item.")

                use_start_line = use_meta_data[use_def_key]['block_start']
                use_end_line = use_meta_data[use_def_key]['block_end']

                use_key = f"{use_name}:{use_file_path}:{use_start_line}:{use_end_line}"
                """

                cashed, use_parent_key, parent_name, parent_block_start = find_parent_def_key(cashed, meta_dir, use_item, use_file_path, use_start_line, program_files)
                uses_list.append(use_parent_key)  #uses_list.append(use_key)

            if key_name not in dependencies:
                dependencies[key_name] = []

            dependencies[key_name] = uses_list #.append(use_key)

    # Call topological_flow_sort()
    #print(dependencies)
    write_json(f"{database_dir}/flow.json", dependencies)
    sorted_functions = topological_flow_sort(dependencies)

    cycles = find_circular_dependencies(dependencies)
    print("Circular dependencies found:")
    for cycle in cycles:
        print(" -> ".join(cycle + [cycle[0]]))

    with open(f'{block_path}', 'w') as f:
        for fun in sorted_functions:
            f.write(f"{fun}\n")

    if len(sorted_functions) == 0:
        raise ValueError(f"sorted_functions must have at least one element.")

    print(f"Saved at {block_path}")



def read_block(file_path, start, end):
    with open(file_path, 'r', encoding='utf-8') as f:
        all_lines = f.readlines()
    return all_lines[start - 1:end]  # 1-indexed


def get_base_name(file_path):
    """Get the base name with the numeric suffix removed from the filename"""
    basename = os.path.basename(file_path)
    name, ext = os.path.splitext(basename)
    # Remove trailing _number
    base = re.sub(r'_\d+$', '', name)
    return base + ext


def get_base_key(file_path):
    """Get the key as the directory path + base name with the numeric suffix removed"""
    dir_path = os.path.dirname(file_path)
    basename = os.path.basename(file_path)
    name, ext = os.path.splitext(basename)
    # Remove trailing _number
    base = re.sub(r'_\d+$', '', name)
    return (dir_path, base + ext)


# Also prepare a map that can be looked up by file path alone (for parsing definition)
# definition is in the format "file_path:line:col"
def replace_definition(definition_str):
    """Replace the file path and line number in the definition string"""
    parts = definition_str.rsplit(":", 2)
    if len(parts) < 3:
        return definition_str, False
    file_path = parts[0]
    line_num = int(parts[1])
    col = parts[2]
    
    key = (file_path, line_num)
    if key in file_line_map:
        kept_file, kept_start = file_line_map[key]
        # Calculate the line offset
        offset = line_num - key[1]  # Relative position within removed
        new_line = kept_start + offset
        return f"{kept_file}:{new_line}:{col}", True
    return definition_str, False


def replace_file_path_and_start(entry):
    """Replace file_path and start_line in an entry of uses/components"""
    fp = entry.get('file_path')
    sl = entry.get('start_line')
    if fp and sl:
        key = (fp, sl)
        if key in file_line_map:
            kept_file, kept_start = file_line_map[key]
            entry['file_path'] = kept_file
            entry['start_line'] = kept_start
    
    # Also replace the definition field
    if 'definition' in entry:
        entry['definition'], _ = replace_definition(entry['definition'])
    
    return entry

    
def rewrite_metadata(meta_dir, replacement_map):
    """
    replacement_map: {(removed_file_path, symbol_name): {
        'kept_file': ...,
        'kept_start': ...,
        'kept_end': ...,
        'removed_start': ...,
        'removed_end': ...,
    }}
    """
    # Build a simple map from removed_file to kept_file
    # Different symbols in the same file do not necessarily point to the same kept file,
    # so use (file_path, start_line) as the key
    file_line_map = {}  # (removed_file, removed_start) -> (kept_file, kept_start)
    for (removed_file, symbol), info in replacement_map.items():
        file_line_map[(removed_file, info['removed_start'])] = (info['kept_file'], info['kept_start'])

    
    # Process all JSON files in meta_dir
    for filename in os.listdir(meta_dir):
        if not filename.endswith('.json'):
            continue
        
        filepath = os.path.join(meta_dir, filename)
        data = read_json(filepath)
        
        new_data = {}
        modified = False
        
        for key, value in data.items():
            new_key = key
            
            # 1. Replace the key itself (format: "name:file_path:line")
            # Extract the file path and line number from the key
            first_colon = key.index(':')
            rest = key[first_colon + 1:]
            rest_parts = rest.rsplit(":", 1)
            if len(rest_parts) == 2:
                key_file = rest_parts[0]
                key_line = int(rest_parts[1])
                if (key_file, key_line) in file_line_map:
                    kept_file, kept_start = file_line_map[(key_file, key_line)]
                    new_key = f"{key[:first_colon]}:{kept_file}:{kept_start}"
                    modified = True
            
            # 2. Own definition
            if 'definition' in value:
                value['definition'], changed = replace_definition(value['definition'])
                if changed:
                    modified = True
            
            # 3. Each entry inside uses
            if 'uses' in value:
                for use_entry in value['uses']:
                    replace_file_path_and_start(use_entry)
                modified = True
            
            # 4. Each entry inside components
            if 'components' in value:
                for comp_key, comp_value in value['components'].items():
                    if isinstance(comp_value, dict):
                        replace_file_path_and_start(comp_value)
                    elif isinstance(comp_value, list):
                        for item in comp_value:
                            if isinstance(item, dict):
                                replace_file_path_and_start(item)
                modified = True
            
            new_data[new_key] = value
        
        if modified:
            write_json(filepath, new_data)


def remove_duplicated_block(meta_dir, div_meta_dir, database_dir, block_path):
    # First, make identical blocks use the first one.
    # Update block_path.
    # 1. Read block_path
    with open(block_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    # 2. Group by symbol name
    symbol_entries = defaultdict(list)  # symbol_name -> [(full_line, file_path, start, end), ...]

    for line in lines:
        line = line.rstrip('\n')
        if not line:
            continue
        # If the beginning is ":" (symbol name is empty)
        if line.startswith(':'):
            parts = line[1:].rsplit(":", 2)
            symbol_name = ""
            file_path = parts[0]
            start_line = int(parts[1])
            end_line = int(parts[2])
        else:
            # "symbol_name:file_path:start_line:end_line"
            first_colon = line.index(':')
            symbol_name = line[:first_colon]
            rest = line[first_colon + 1:]
            parts = rest.rsplit(":", 2)
            file_path = parts[0]
            start_line = int(parts[1])
            end_line = int(parts[2])
        
        symbol_entries[symbol_name].append({
            'line': line,
            'file_path': file_path,
            'start': start_line,
            'end': end_line,
        })
    
    # 3. Compare code contents for each symbol and remove duplicates
    # Result after removing duplicates
    kept_lines = []
    # Replacement map: removed file path -> file path to keep
    replacement_map = {}  # (removed_file, symbol) -> kept_file
    
    # Inspection records
    duplicated_records = []   # Identical ones
    different_records = []    # Same symbol and same base name, but different contents

    """
    for symbol_name, entries in symbol_entries.items():
        if len(entries) <= 1:
            kept_lines.append(entries[0]['line'])
            continue
        
        # Group by code content
        groups = []  # [(code_content, [entries...])]
        for entry in entries:
    """
    
    for symbol_name, entries in symbol_entries.items():
        if len(entries) <= 1:
            kept_lines.append(entries[0]['line'])
            continue
        
        # First, subgroup by the base name of the file
        base_groups = defaultdict(list)
        for entry in entries:
            # base = get_base_name(entry['file_path'])
            # base_groups[base].append(entry)
            base_key = get_base_key(entry['file_path'])
            base_groups[base_key].append(entry)

        
        for base, base_entries in base_groups.items():
            if len(base_entries) <= 1:
                kept_lines.append(base_entries[0]['line'])
                continue
            
            # Group by code content
            groups = []
            for entry in base_entries:
                code = read_block(entry['file_path'], entry['start'], entry['end'])
                code_str = ''.join(code)
                
                found = False
                for group_code, group_entries in groups:
                    if group_code == code_str:
                        group_entries.append(entry)
                        found = True
                        break
                if not found:
                    groups.append((code_str, [entry]))
            
            """
            # Keep only the first entry of each group
            for code_str, group_entries in groups:
                kept_lines.append(group_entries[0]['line'])
                # Record the second and subsequent entries as removal targets
                for removed in group_entries[1:]:
                    replacement_map[(removed['file_path'], symbol_name)] = group_entries[0]['file_path']
            """
            # Keep only the first entry of each group
            for code_str, group_entries in groups:
                kept_lines.append(group_entries[0]['line'])
                # Record the second and subsequent entries as removal targets
                for removed in group_entries[1:]:
                    replacement_map[(removed['file_path'], symbol_name)] = {
                        'kept_file': group_entries[0]['file_path'],
                        'kept_start': group_entries[0]['start'],
                        'kept_end': group_entries[0]['end'],
                        'removed_start': removed['start'],
                        'removed_end': removed['end'],
                    }
                    duplicated_records.append({
                        'symbol': symbol_name,
                        'kept': group_entries[0]['file_path'],
                        'kept_start': group_entries[0]['start'],
                        'kept_end': group_entries[0]['end'],
                        'removed': removed['file_path'],
                        'removed_start': removed['start'],
                        'removed_end': removed['end'],
                    })
            
            # If there are multiple groups with different contents, record them
            if len(groups) > 1:
                different_records.append({
                    'symbol': symbol_name,
                    'base': base,
                    'groups': [
                        [e['file_path'] for e in group_entries]
                        for _, group_entries in groups
                    ],
                })

    # 4. Update block_path
    with open(block_path, 'w', encoding='utf-8') as f:
        for line in kept_lines:
            f.write(line + '\n')
    
    write_json(os.path.join(database_dir, "duplicated_blocks.json"), duplicated_records)
    write_json(os.path.join(database_dir, "different_blocks.json"), different_records)

    # Next, fix the relevant parts in meta_dir and div_meta_dir

    rewrite_metadata(meta_dir, replacement_map)
    rewrite_metadata(div_meta_dir, replacement_map)



def separate_enumerator(c_path, raw_dir, meta_dir): # It is unclear how much influence the uncategorized missed ranges (missed by ctags and libclang) have.
    # Sort based on cflow_order
    meta_data, meta_path = obtain_metadata(c_path, meta_dir, False, None, "def")
    
    insert_list = []
    for item in meta_data:
        if 'enumerator' in item:
            for enum in item['enumerator']:
                insert_item = item.copy()
                insert_item['name'] = enum
                del insert_item['enumerator']
                insert_list.append(insert_item)
    
    meta_data.extend(insert_list)

    write_json(meta_path, meta_data)


def find_minimal_circular_dependencies(json_data):
    # Build the dependency graph
    dependency_graph = {}
    for item in json_data:
        source = item["source"]
        indirect_includes = item.get("indirect_include", [])
        dependency_graph[source] = indirect_includes
    
    def find_cycles(node, path=None, visited=None):
        if path is None:
            path = []
        if visited is None:
            visited = set()
        
        # Early return if the node has already been visited
        if node in visited and node not in path:
            return []
            
        path.append(node)
        visited.add(node)
        
        cycles = []
        for neighbor in dependency_graph.get(node, []):
            if neighbor in path:
                # Found a circular dependency - extract only the minimal cycle
                cycle = path[path.index(neighbor):]
                cycle.append(neighbor)
                cycles.append(cycle)
            else:
                # Create only a copy of path, and share visited
                new_cycles = find_cycles(neighbor, path.copy(), visited)
                cycles.extend(new_cycles)
        
        path.pop()  # Remove the current node from the path during backtracking
        return cycles
    
    # Collect all circular dependencies
    all_cycles = []
    seen_cycles = set()
    
    # Track visited nodes
    global_visited = set()
    
    for start_node in dependency_graph:
        if start_node not in global_visited:
            cycles = find_cycles(start_node)
            global_visited.add(start_node)
            
            for cycle in cycles:
                # Normalize the cycle
                min_index = cycle.index(min(cycle))
                normalized_cycle = cycle[min_index:-1] + cycle[:min_index] + [cycle[min_index]]
                cycle_str = " -> ".join(normalized_cycle)
                
                # Optimize subset checking
                if not any(set(cycle_str.split(" -> ")).issubset(set(seen.split(" -> "))) or 
                          set(seen.split(" -> ")).issubset(set(cycle_str.split(" -> "))) 
                          for seen in seen_cycles):
                    seen_cycles.add(cycle_str)
                    all_cycles.append(normalized_cycle)
    
    # Sort by cycle length
    all_cycles.sort(key=len)
    return all_cycles


def find_connected_dependency_sets(cycles):
    # Convert each cycle to a set (remove duplicates at the end)
    cycle_sets = [set(cycle) for cycle in cycles]
    
    # Merge sets that contain the same file
    merged = True
    while merged:
        merged = False
        new_cycle_sets = []
        used = set()
        
        for i, set1 in enumerate(cycle_sets):
            if i in used:
                continue
                
            current_set = set1
            used.add(i)
            
            # Check intersections with other sets
            for j, set2 in enumerate(cycle_sets):
                if j in used:
                    continue
                    
                # Merge if there are common elements
                if current_set & set2:
                    current_set |= set2
                    used.add(j)
                    merged = True
            
            new_cycle_sets.append(current_set)
        
        if merged:
            cycle_sets = new_cycle_sets
    
    return cycle_sets


def sort_by_source(list_path, dep_json_path):
    dep_json = read_json(dep_json_path)

    dependencies = {}
    
    # Build the graph and create the file list
    for dep in dep_json:
        source = dep['source']
        dependencies[source] = dep['indirect_include']
    
    sorted_files = topological_flow_sort(dependencies)
    #sorted_files, cycles = topological_file_sort(dependencies)

    cycles = find_minimal_circular_dependencies(dep_json) #find_circular_dependencies(dependencies)
    print("Circular dependencies found:")
    for cycle in cycles:
        print(" -> ".join(cycle + [cycle[0]]))

    dependency_sets = find_connected_dependency_sets(cycles)
    # Display the results
    print("Connected dependency sets:")
    for i, dep_set in enumerate(dependency_sets, 1):
        print(f"\nSet {i}:")
        print(", ".join(sorted(dep_set)))


    with open(list_path, 'w') as f:
        for file_path in sorted_files:
            f.write(f"{file_path}\n")

    with open(f'{database_dir}/sorted_flies.txt', 'w') as f:
        for file_path in sorted_files:
            f.write(f"{file_path}\n")
    


def output_block_order(block_path, sorted_func_path, sorted_type_path):

    with open(block_path, 'w') as output_file:
        # First read from sorted_type_path and write it
        if os.path.exists(sorted_type_path):
            with open(sorted_type_path, 'r') as type_file:
                for line in type_file:
                    output_file.write(line)
        
        # Next read from sorted_func_path and write it
        if os.path.exists(sorted_func_path):
            with open(sorted_func_path, 'r') as func_file:
                for line in func_file:
                    output_file.write(line)


##############################################
##### Pre-processing Start!
##############################################

def summarize_components_with_keys(file_path, raw_dir, meta_dir, keep_key_reference=True):
    """
    Version that keeps components by key reference only
    (stores only keys instead of saving complete objects)
    
    Args:
        file_path: Path to the target source file
        raw_dir: Directory of raw files
        meta_dir: Directory of metadata
        keep_key_reference: If True, store only keys in components
    
    Returns:
        int: Number of hierarchical elements
    """
    meta_data, meta_path = obtain_metadata(file_path, meta_dir, False, None, "def")
    
    # Initialize the components field for each item
    for key, item in meta_data.items():
        if keep_key_reference:
            item['components'] = []  # List of keys
        else:
            item['components'] = {}  # Dictionary of complete objects
    
    # Track nested elements
    nested_item_keys = set()
    
    # Convert metadata to a list
    items_list = list(meta_data.items())
    
    # Analyze containment relationships
    for i, (key_i, item) in enumerate(items_list):
        start_line = item.get('start_line', 0)
        end_line = item.get('end_line', start_line)
        uses = set(item.get('uses', []))
        
        for j, (key_j, other_item) in enumerate(items_list):
            if i == j:
                continue
            
            other_start_line = other_item.get('start_line', 0)
            other_end_line = other_item.get('end_line', other_start_line)
            
            # Containment check
            if start_line <= other_start_line and other_end_line <= end_line:
                if not (start_line == other_start_line and end_line == other_end_line):
                    if keep_key_reference:
                        # Save only the key
                        if key_j not in item['components']:
                            item['components'].append(key_j)
                            nested_item_keys.add(key_j)
                    else:
                        # Save the complete object
                        if key_j not in item['components']:
                            item['components'][key_j] = dict(other_item)
                            nested_item_keys.add(key_j)
                    
                    # Merge uses
                    # other_uses = set(other_item.get('uses', []))
                    # uses.update(other_uses)

                    # === Merge uses (while preserving the dictionary structure) ===
                    other_uses_list = other_item.get('uses', [])
                    
                    for other_use in other_uses_list:
                        other_use_name = other_use.get('name')
                        if other_use_name and other_use_name not in uses_dict:
                            uses_dict[other_use_name] = other_use
                    
                    uses.update(other_uses)
        
        # Update uses
        item['uses'] = list(uses)
    
    # Remove nested elements from the top level
    top_level_items = {
        key: item 
        for key, item in meta_data.items() 
        if key not in nested_item_keys
    }
    
    # Write the updated metadata
    write_json(meta_path, top_level_items)
    
    return len(nested_item_keys)


def get_target_location(target_dir, target_path): #, marker):
    """
    Parse function information from a text file and return it as JSON
    
    Args:
        target_path: Path to the text file containing function information
        
    Returns:
        List of dictionaries in JSON format
    """
    functions = []
    
    with open(target_path, 'r') as f:
        content = f.read()
    
    entries = content.split()
    
    for entry in entries:
        parts = entry.split(':')
        
        if len(parts) >= 4:
            functions.append({
                'name': parts[0],
                'file': f"{target_dir}/{parts[1]}",
                'start_line': int(parts[2]),
                'end_line': int(parts[3])
            })
    
    print(f"Parsed {len(functions)} item(s)")
    
    # Search for the actual locations in target_dir where the following comment has been inserted:
    # self.marker = f'/* Genifai: here is one target function!: [file_path:line:func_name] */'
    # Marker pattern: /* Genifai: here is one target function!: [file_path:line:func_name] */
    #marker_pattern = re.compile(r'/\* Genifai: here is one target function!: \[([^:]+):(\d+):([^\]]+)\] \*/')
    #marker_pattern = re.compile(r'/\* Genifai: here is one target function!: ([^:]+):(\d+):(\d+):([^\s]+) \*/')
    marker_pattern = re.compile(r'/\* Genifai: here is one target function!: (.+):(\d+):(\w+) \*/')

    actual_locations = []
    
    for root, dirs, files in os.walk(target_dir):
        for filename in files:
            if filename.endswith(('.c', '.h', '.cpp', '.cc')):
                file_path = os.path.join(root, filename)
                abs_path = os.path.abspath(file_path)
                
                with open(file_path, 'r') as f:
                    file_lines = f.readlines()
                
                for line_num, line in enumerate(file_lines, start=1):
                    # if 'Genifai' in line:
                    #     print(f"Found Genifai in {file_path}:{line_num}")
                    #     print(f"  Line: {line.strip()}")
                    #     match = marker_pattern.search(line)
                    #     print(f"  Match: {match}")
                    #     if match:
                    #         print(f"  Groups: {match.groups()}")

                    match = marker_pattern.search(line)
                    if match:
                        orig_file = match.group(1)
                        orig_line = int(match.group(2))
                        #orig_column = int(match.group(3))
                        func_name = match.group(3)
                        
                        actual_locations.append({
                            'original_file': orig_file,
                            'original_line': orig_line,
                            #'original_column': orig_column,
                            'name': func_name,
                            'actual_file': abs_path,
                            'actual_start_line': line_num
                        })
    
    print(f"Found {len(actual_locations)} marker(s)")

    # Write to output_target_path (same format as target_path: name:file:line:column)
    output_target_path = target_path.replace("targets.txt", "targets_actual.txt")
    with open(output_target_path, 'w') as f:
        for loc in actual_locations:
            # Since the original marker position is unknown, set column to 1
            f.write(f"{loc['name']}:{loc['actual_file']}:{loc['actual_start_line']}\n")
    
    print(f"Wrote {len(actual_locations)} location(s) to {output_target_path}")

    return output_target_path


def generate_build_setup(taken_macros_path, independent_const_build_path, flag_build_path):

    macros = read_json(taken_macros_path)
    ind_consts = set() #[]
    flags = set() #[]

    for macro_key, item in macros.items():
        if item['is_const'] is True and item['is_independent'] is True:
            ind_consts.add(item['name'])

        if item['is_flag'] is True:
            flags.add(item['name'])

    independents = {}
    independents['independent'] = list(ind_consts)
    write_json(independent_const_build_path, independents)

    flag_macros = {}
    flag_macros['flags'] = list(flags)
    write_json(flag_build_path, flag_macros)


def pre_processing(analyzer_path, macro_analyzer_path, target, original_dir, target_dir, meta_dir, div_meta_dir, database_dir, dep_json_path, # raw_dir,  
                        all_directive_path, taken_directive_path, c_run_path, cfg_path, independent_path, flag_path, is_program_path,  # , all_macro_path, compile_log_path
                        all_macros_path, taken_macros_path, guards_path, list_path, block_path, target_path, macro_finder,
                        guarded_macros_path, const_path, build_path, given_compile_dir, given_compile_json_path,
                        independent_const_build_path, flag_build_path, global_path): #, llm_on): # c_output_dir, #  updated_json_path,
    
    # This is duplicated here, so I think it can be removed eventually
    # taken_directive_path = f"{database_dir}/all_directive_def.json"
    unordered_taken_directive_path = f"{database_dir}/unordered_taken_directive.json"

    parse_all("all", macro_finder, target_dir, meta_dir, div_meta_dir, database_dir, build_path, 
                    taken_directive_path, unordered_taken_directive_path, all_directive_path, dep_json_path, is_program_path, 
                    all_macros_path, taken_macros_path, guards_path, guarded_macros_path, independent_path, flag_path, const_path,
                    given_compile_dir, given_compile_json_path, global_path) # , cfg_path
 
    """
    build_path = get_build_path(target_dir)
    run_script_wo_log(build_path, 1000, True, None, "build")
    build_dir = find_compile_commands_json(target_dir) # raw_dir is false
    print(build_dir)
    ##
    # compile_log_path = f'{database_dir}/compile.log'
    compile_dir, compile_json_path = get_compile_json(target_dir)
    """

    ################################################
    # Newly insert locations to be partially expanded
    #changed = insert_expanded_code(target_dir, meta_dir, database_dir)

    changed = False
    if changed is True:
        # 4th round: parsing
        parse_all("4", macro_finder, target_dir, meta_dir, div_meta_dir, database_dir, build_path, 
                    taken_directive_path, unordered_taken_directive_path, all_directive_path, dep_json_path, is_program_path, 
                    all_macros_path, taken_macros_path, guards_path, guarded_macros_path, independent_path, flag_path, const_path, 
                    given_compile_dir, given_compile_json_path, global_path) # , cfg_path

    """
    # It is necessary to detect the consolidated conditional blocks of the expanded parts
    # Merge conditional blocks
    changed = change_combined_condition(target_dir, meta_dir, database_dir, unordered_taken_directive_path)

    if changed is True:
        # 4th round: parsing
        parse_all("4", macro_finder, target_dir, meta_dir, div_meta_dir, database_dir, build_path, 
                    taken_directive_path, unordered_taken_directive_path, all_directive_path, dep_json_path, is_program_path, 
                    all_macros_path, taken_macros_path, guards_path, guarded_macros_path, independent_path, flag_path, const_path) # , cfg_path
    """

    ################################################
    """
    #analyze_dependencies(target_dir, dep_json_path, c_run_path, compile_log_path, build_dir, database_dir) # , list_path, omitted_files) # f"{raw_dir}/{target}"
    find_headers(target_dir, database_dir, dep_json_path, compile_dir, compile_json_path, None)
    """

    # obtain the parse data  # c_create_defdata()
    # get_list_path(dep_json_path, target_dir, list_path)
    #parse_files_c(meta_dir, raw_dir, target, database_dir, dep_json_path, True, False, c_run_path, all_macros_path, list_path) # flag_file, 
    
    # detect_independent_macros(unique_macros, independent_path) # I put this inside generate_macro_data, though

    """
    # Write out the cfg-related items here (macros used for conditional compilation) <- remove include guards
    detect_cfg(unordered_taken_directive_path, guards_path, cfg_path)  #detect_cfg(all_directive_path, guards_path, cfg_path)

    # Make prompt generation easier by incorporating cfg if statements as component elements
    #insert_ifdef_statement(cfg_path, target_dir, meta_dir) # flag_path may have been cfg_path
    insert_ifdef_statement(cfg_path, target_dir, div_meta_dir) # flag_path may have been cfg_path
    """

    # parse_all("4", macro_finder, target_dir, meta_dir, div_meta_dir, database_dir, build_path, 
    #              taken_directive_path, unordered_taken_directive_path, all_directive_path, dep_json_path, 
    #              all_macros_path, taken_macros_path, guards_path, independent_path, flag_path)
    
    #---------------------------------------------
    # Determine blocks # This makes various things disappear, though... I thought I had included it in parse_all
    #define_blocks(None, all_directive_path, guards_path, target_dir, meta_dir, div_meta_dir, database_dir)  # , raw_dir


    #---------------------------------------------
    # Remove include guards # Is it okay to do this after define_blocks()? define_blocks() removes them virtually
    delete_guards(guards_path, target_dir, meta_dir, is_program_path)  

    """
    # Try turning this off for now.
    # Since definitions are handled in the configuration section, remove all definition locations for conditional macros
    # Comment-associated items should remain: it may be good to leave notation such as // deleted
    # This will remove definitions of all macros, but perhaps macros are indeed unnecessary now that they are inherited from the C world
    delete_independent_defs(independent_path, target_dir, is_program_path) #delete_macro_defs(all_macros_path, target_dir)s
    # ↑The state macros for if should be nested and removed here, though: delete_state_macro_defs()

    # Remove declaration locations of global variables
    delete_global_defs(target_dir, meta_dir, is_program_path)
    """

    #################
    # Replace the conversion numbers whose boundaries have been determined
    #order = get_files_list(list_path)
    define_block_order(meta_dir, div_meta_dir, target_dir, database_dir, is_program_path, block_path) #order)  # raw_dir,  #change_block_order(raw_dir, meta_dir)

    remove_duplicated_block(meta_dir, div_meta_dir, database_dir, block_path)

    # copy the dir to the initial/parent dir
    # clone_directory(raw_dir, root_dir)

    # Create independenc_const_build_path and flag_build_path
    generate_build_setup(taken_macros_path, independent_const_build_path, flag_build_path)

    # Find the location of the target function
    out_taget_path = get_target_location(target_dir, target_path) # , marker

    print(f"\n+++++++ End of parse_files_c() +++++++\n")


    """
    # Detect sys macros
    #detect_sys_macros(macro_metadata, sys_macros_path)
    """


def get_setting_data(data, target_dir):  # , target # translation_dir, 
    # dst_dir = f"{translation_dir}/{target}"
    # print(dst_dir)
    # config_path = f"{dst_dir}/setting.json"

    created_paths = [] #data['created_paths']  # This is dangerous, so
    if data is None:
        data = {}

    build_path = data['build_path']
    run_test_path = data['run_test_path']
    run_all_path = data['run_all_path']
    target_funcs = data['target_funcs']

    print(f"created_paths: {created_paths}")
    print(f"build_path: {build_path}")
    print(f"run_test_path: {run_test_path}")
    print(f"run_all_path: {run_all_path}")

    # filetered_created_paths = []
    # print(f"file_paths in created_paths")
    # for file_path in created_paths:
    #     file_path = f"{target_dir}/{file_path}"
    #     filetered_created_paths.append(file_path)
    #     print(f"{file_path}")

    filetered_target_funcs = []
    print(f"file_paths in target_funcs")
    for item in target_funcs:
        file_path = item['def_file_path']
        item['def_file_path'] = f"{target_dir}/{file_path}"
        filetered_target_funcs.append(item)
        print(f"{file_path}")


    build_path = f"{target_dir}/{build_path}"
    run_test_path = f"{target_dir}/{run_test_path}"
    run_all_path = f"{target_dir}/{run_all_path}"

    #write_json(config_path, data)
    return build_path, run_test_path, run_all_path, filetered_target_funcs # filetered_created_paths,  # , run_all_path #created_paths




def get_created_c_paths(translation_dir, target):
    dst_dir = f"{translation_dir}/{target}"
    config_path = f"{dst_dir}/setting.json"

    data = read_json(config_path)

    created_paths = data['created_paths']
    build_path = data['build_path']
    run_test_path = data['run_test_path']
    run_all_path = data['run_all_path']

    return created_paths


def remove_created_c_paths(list_path, created_paths):
    order = get_compile_order(list_path)

    filtered_order = []
    for file_path in order:
        if file_path not in created_paths:
            filtered_order.append(file_path)
    
    write_json(list_path, filtered_order)
    return filtered_order



# Not need in case we use as a CLI tool
def handle_paths(all_macros_path, taken_macros_path, all_directive_path, taken_directive_path, guards_path, independent_path, is_program_path, dep_json_path, compile_json_path): # , cfg_path
    #guards_path = "database_0000/mini/guards.json"

    paths = [all_macros_path, taken_macros_path, all_directive_path, taken_directive_path, guards_path, independent_path, is_program_path, dep_json_path, compile_json_path]
    paths = [f"{MACRO_HOME}/{item}".replace("trans", "macro") for item in paths]
    all_macros_path, taken_macros_path, all_directive_path, taken_directive_path, guards_path, independent_path, is_program_path, dep_json_path, compile_json_path = paths

    return all_macros_path, taken_macros_path, all_directive_path, taken_directive_path, guards_path, independent_path, is_program_path, dep_json_path, compile_json_path


def initialize(target_dir, meta_dir, database_dir, dep_json_path): #, flag_json_path, macro_list_path, all_macros_path, initial_macro_path, all_macro_path):

    # initialize directories and files
    #delete_directory(raw_dir)
    delete_directory(target_dir)
    delete_directory(meta_dir)
    #delete_directory(root_dir)

    #delete_directory("preprocessed_output")

    #delete_file(macro_list_path)
    #delete_file(all_macros_path)
    #delete_file(initial_macro_path)

    #delete_file(initial_list_path)
    #delete_file(all_macro_path)

    delete_file(f'{database_dir}/m_conds.json')
    delete_file(f'{database_dir}/m_grep.json')

    delete_file(f"{database_dir}/dep_user.json")
    #delete_file(token_path)
    #delete_file("token_macro.json")

    delete_file(f'{database_dir}/macro_func.txt')

    #delete_file('find_defines.c') # Not confirmed yet
    #delete_file('find_if.c')  # Not confirmed yet

    # create new directories
    if not DEBUG_LLM:
        delete_directory(database_dir)
        create_directory(database_dir)

    write_json(f"{database_dir}/pro_functions.json", {})
    write_json(f"{database_dir}/pro_data_type.json", {})
    write_json(f"{database_dir}/pro_global_var.json", {})
    write_json(f"{database_dir}/pro_macro.json", {})

    write_json(f"{database_dir}/pro_pro_functions.json", {})
    write_json(f"{database_dir}/pro_pro_data_type.json", {})
    write_json(f"{database_dir}/pro_pro_global_var.json", {})
    write_json(f"{database_dir}/pro_pro_macro.json", {})

    # write_json("pro_pro_functions.json", [])
    # write_json("pro_pro_data_type.json", [])
    # write_json("pro_pro_global_var.json", [])
    # write_json("pro_pro_macro.json", [])



def merge_different_meta_dir(target_dir, old_meta_dir, meta_dir, old_div_meta_dir, div_meta_dir):

    file_paths = get_all_files(target_dir)

    for file_path in file_paths:
        meta_data, meta_path = obtain_metadata(file_path, meta_dir, False, None, "def")
        old_meta_data, old_meta_path = obtain_metadata(file_path, old_meta_dir, False, None, "def")

        for key, value in old_meta_data.items():
            if key not in meta_data:
                meta_data[key] = value

        write_json(meta_path, meta_data)

        
        div_meta_data, div_meta_path = obtain_metadata(file_path, div_meta_dir, False, None, "def")
        old_div_meta_data, old_div_meta_path = obtain_metadata(file_path, old_div_meta_dir, False, None, "def")

        for key, value in old_div_meta_data.items():
            if key not in div_meta_data:
                div_meta_data[key] = value

        write_json(div_meta_path, div_meta_data)


def allrust_preprocess_main(config):

    ############################################
    ##### Configuration
    ############################################

    process_type = config["process_type"]
    user_id = config["user_id"]
    original_dir = config["original_dir"]
    target_path = config["target_path"]
    given_meta_dir = config["meta_dir"]
    given_div_meta_dir = config["div_meta_dir"]
    given_compile_dir = config["compile_dir"]

    llm_choice = config["llm_choice"]
    claude_api_key = config["claude_api_key"]
    azure_endpoint = config["azure_endpoint"]


    #llm_on = get_llm_flag(llm_on)
    claude_model = get_claude_model(llm_choice)

    occupy_path = None
    #config_path = f"{MACRO_HOME}/config.json"
    #config_data = read_json(config_path)

    analyzer_path = f"{C_PARSER_HOME}/c_analyzer/analyzer"  # or absolute path
    macro_analyzer_path = f"{MACRO_HOME}/macro_analyzer/build/macro_analyzer"


    # Create PathConfig
    paths = create_path_config(
        user_id=user_id,
        original_dir=original_dir,
        process_type=process_type,
        work_dir=None,
        #target="mini",
        #def_json_path=def_json_path,
    )

    (target,
    build_path, 
    rust_build_path,
    rust_lib_h_path,
    run_test_path,
    run_all_path,
    raw_dir, #
    target_dir, 
    work_dir, 
    c_code_dir,
    rust_output_dir, 
    execute_path,
    marker,

    meta_dir, 
    div_meta_dir,
    chat_dir, 
    chat_macro_dir, 
    log_dir, 
    exp_dir,
    archive_dir, 

    macro_finder, 
    database_dir, 
    #lib_path, 

    dep_json_path, 
    list_path, 
    result_path, 
    moment_path,
    line_path, 
    logging_path,
    
    guards_path, 
    guarded_macros_path,
    taken_macros_path,
    all_macros_path,
    taken_directive_path,
    all_directive_path,
    cfg_path,
    independent_path,
    flag_path,
    const_path,
    conflict_path,
    global_path,
    is_program_path,
    build_config_path,
    
    custom_headers_dir,
    custom_json_path,
    custom_header_path,

    block_path, 
    block_group_path,
    rust_c_path,
    c_rust_path,

    map_path, 
    call_path, 
    persistent_dir, 
    #build_rs_path, 

    chat_dir,
    history_path,
    token_path,
    count_path, 
    time_path,
    output_dir,
    
    independent_const_build_path, 
    flag_build_path) = extract_all_paths(paths)


    ############################################
    if process_type == "meta":

        #******************************************************************
        #*******       Setup       
        #******************************************************************

        start_time = time.time()

        # initialize
        initialize(target_dir, meta_dir, database_dir, dep_json_path) #, flag_json_path, macro_list_path, all_macros_path, initial_macro_path, all_macro_path)

        # copy the target directory
        print(f"original_dir: {original_dir}") 
        copy_directory(original_dir, raw_dir)
        grant_permissions(target_dir) 

        # print(run_test_path)
        # print(run_all_path)
        # print(build_path)

        # I don't think it's needed in the real environment; only for testing
        compile_json_path = get_compile_json(target_dir)
        # all_macros_path, taken_macros_path, all_directive_path, taken_directive_path, guards_path, independent_path, is_program_path, v_dep_json_path, compile_json_path = handle_paths(all_macros_path, taken_macros_path, all_directive_path, taken_directive_path, guards_path, independent_path, is_program_path, None, compile_json_path) # , cfg_path
        #
        print(original_dir)
        # print(os.path.abspath(target_dir))

        recreate_directory(meta_dir)
        recreate_directory(div_meta_dir)

        clone_directory(given_meta_dir, meta_dir)
        clone_directory(given_div_meta_dir, div_meta_dir)

        denormalize_translation_metadata(meta_dir, os.path.abspath(target_dir), False) #os.path.abspath(target_dir))
        denormalize_translation_metadata(div_meta_dir, os.path.abspath(target_dir), False) #, os.path.abspath(target_dir))

        given_compile_json_path = setup_compile_json(given_compile_dir, f"{MACRO_HOME}", f"{TRANS_HOME}")

        #denormalize_dep_data(dep_json_path, f"{TRANS_HOME}", os.path.abspath(f"{TRANS_HOME}")) #, os.path.abspath(target_dir))
        
        denormalize_metafiles(meta_dir, raw_dir, all_macros_path, taken_macros_path, guards_path)
        #denormalize_block_path(is_program_path, f"{MACRO_HOME}", f"{TRANS_HOME}")

        # print(target_dir)
        # print(c_code_dir)
        if os.path.exists(f"{c_code_dir}/{target}"):
            delete_directory(f"{c_code_dir}/{target}") # Initialize
        copy_directory(target_dir, c_code_dir)

        #******************************************************************
        #*******       Pre-processing      
        #******************************************************************
        
        # start preprocessing # c_run_path = 'raw/which-2.21/build.sh'  # flag_json_path, list_path, picked_path, namespace_path, c_lib_path, c_build_path, c_cargo_path, 
        pre_processing(analyzer_path, macro_analyzer_path, target, original_dir, target_dir, meta_dir, div_meta_dir, database_dir, dep_json_path,  # raw_dir, 
                        all_directive_path, taken_directive_path, run_test_path, cfg_path, independent_path, flag_path, is_program_path, # all_macro_path, compile_log_path, 
                        all_macros_path, taken_macros_path, guards_path, list_path, block_path, target_path, macro_finder,
                        guarded_macros_path, const_path, build_path, given_compile_dir, given_compile_json_path,
                        independent_const_build_path, flag_build_path, global_path) #, llm_on) # created_paths,   # c_output_dir,  # updated_json_path

        
        #******************************************************************
        #*******       Completion process       
        #******************************************************************
       
        print(raw_dir)
        normalize_translation_metadata(meta_dir, f"{TRANS_HOME}/trans_c_0000")
        normalize_translation_metadata(div_meta_dir, f"{TRANS_HOME}/trans_c_0000")

        # Should I revert the metafiles as well? (Although I don't think it's necessary in the actual API)
        normalize_metafiles(meta_dir, raw_dir, all_macros_path, taken_macros_path, guards_path)        

        #　normalize_dep_data(dep_json_path, raw_dir)
        # remove_unordered_meta(meta_dir, raw_dir, target)

        end_time = time.time()
        elapsed_time = end_time - start_time

        output = {
            'meta_dir' : meta_dir,
            'mid_dir' : target_dir,
            'meta_dir' : c_code_dir,
        }

        print(f"\nExecution time: {elapsed_time:.2f} seconds")
        print(f"block_path: {block_path}")
        print(f"************ pre-process finished ************")
        
        print(f"\nNext action:")
        print(f"\npython3 compile.py {TRANS_HOME}/{c_code_dir}/{target} {TRANS_HOME}/{target_dir} /root/SmartC2Rust/benchmark/{target}/targets_actual.txt trans {os.path.abspath(meta_dir)} {os.path.abspath(div_meta_dir)} {block_path} off")


    if process_type == "merge":

        old_meta_dir = config["old_meta_dir"]
        old_div_meta_dir = config["old_div_meta_dir"]
        merge_different_meta_dir(old_meta_dir, meta_dir, old_div_meta_dir, div_meta_dir)

        print(f"\nNext action:")
        print(f"\npython3 compile.py {TRANS_HOME}/{c_code_dir}/{target} {TRANS_HOME}/{target_dir} /root/SmartC2Rust/benchmark/{target}/targets_actual.txt trans {os.path.abspath(meta_dir)} {os.path.abspath(div_meta_dir)} {block_path} off")
        



if __name__ == "__main__":

    #####################################################################
    ##### Input
    #####################################################################
   
    original_dir = str(sys.argv[1])
    process_type = str(sys.argv[2])
    target_path = str(sys.argv[3])
    meta_dir = str(sys.argv[4])
    div_meta_dir = str(sys.argv[5])
    compile_dir = str(sys.argv[6])

    old_meta_dir = None
    old_div_meta_dir = None
    if process_type == "resume":
        old_meta_dir = str(sys.argv[7])
        old_div_meta_dir = str(sys.argv[8])

    # llm_on = str(sys.argv[3]) # process_type = "meta"
    # target = str(sys.argv[1])

    user_id = "0000"
    llm_choice = None
    claude_api_key = None
    azure_endpoint = None

    config = {
        "original_dir": original_dir,
        "process_type": process_type,
        "target_path": target_path,
        "meta_dir": meta_dir,
        "div_meta_dir": div_meta_dir,
        "compile_dir" : compile_dir,
        "old_meta_dir" : old_meta_dir,
        "old_div_meta_dir" : old_div_meta_dir,
        "user_id": user_id,
        "llm_choice": llm_choice,
        "claude_api_key": claude_api_key,
        "azure_endpoint": azure_endpoint,
    }
    allrust_preprocess_main(config) #process_type, user_id, original_dir, target_path, llm_choice, claude_api_key, azure_endpoint)
        
