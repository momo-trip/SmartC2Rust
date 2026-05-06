import os
import json
import sys
import subprocess
import shutil
import logging
import base64
import atexit
import signal
import random
import math
import time
import tempfile
import concurrent.futures
from concurrent.futures import ThreadPoolExecutor
import tiktoken
import chardet
import requests
import threading
import http.server
import socketserver
import traceback
from functools import partial, reduce
import select
import platform
import webbrowser
import stat
import pwd
import grp
import glob
import fcntl
import pty
import termios
import tty
import argparse
import re
import textwrap
import toml
from pathlib import Path
from typing import Dict, List, Tuple, Set, Any, Optional, Union, NamedTuple
from copy import deepcopy
from collections import defaultdict, deque
from datetime import datetime, timedelta
from threading import Timer, Thread
from dataclasses import dataclass, field
import subprocess
from pathlib import Path
import networkx as nx
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
from pydantic import BaseModel
import openai
import anthropic
from anthropic import InternalServerError
import replicate
# import google.generativeai as genai
# from google.generativeai.protos import Content, Part

from clang.cindex import (
    Index, 
    CursorKind, 
    TypeKind, 
    TranslationUnit, 
    CompilationDatabase, 
    Config
)
from graphviz import Digraph
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import clang.cindex
clang.cindex.Config.set_library_file('/usr/lib/llvm-19/lib/libclang.so.1')  # Numbers may change depending on version
# clang.cindex.Config.set_library_file('/opt/homebrew/opt/llvm/lib/libclang.dylib') # for mac os


from utils_api import (
    # normal
    read_json,
    write_json,
    read_file,
    write_file,
    delete_file,
    create_file, 
    copy_file,
    append_file,
    create_permissioned_file,
    create_directory,
    delete_directory,
    copy_directory,
    grant_permissions,
    run_script,
    run_cov_script,
    run_branch_cov_script,
    get_coverage,
    get_branch_covered,
    find_compile_commands_json,
    deduplicate_compile_commands,
    count_file_lines,
    get_timestamp,
    write_testcase,
    remove_base_path,
    calculate_execution_time,
    rename_directory,
    get_last_directory,
    create_backup_directory,
    run_script_wo_log,
    run_script_pty,

    # translation
    set_log,
    create_path_config,
    extract_all_paths,
    get_setting_data,
    get_lined_code,
    update_parent_path,
    obtain_metadata,
    get_ref_files,
    get_path_map,
    read_specific_lines,
    get_lined_specific_code,
    append_rust_path,
    update_modified_keys,
)

from llm_api import (
    SemConfig,
    LLMInterface,
    init_prompt_count, 
    occupy_llm,
    configure_llm,
    shutdown_llm,
    save_coverage_report,
    get_dir_struct,
    adjust_prompt,
    ask_llm,
    reflect_line_modification,
    get_modified_rust_lines,
    get_grouped_c_keys,
    ask_correspondence,
    get_claude_model,
    adjust_prompt,
    check_excluded,
    is_empty_string,
    calc_claude_cost_from_file,
)

from c_parser_api import (
    analyze_dependencies,
    analyze_call_relationship,
    p_f,
    get_files_list,
    detect_include_guards,
    delete_guards,
    delete_macro_defs,
    generate_cargo_toml,
    generate_run_all_path,
    generate_header_paths_rust_code,
    get_headers,
    get_build_path,
    parse_trace,
    find_binaries,
)

from rust_parser_api import (
    parse_files_rust,
    get_rust_interface,
    rust_find_function_end,
    update_c_rust_metadata,
    merge_c_rust_metadata,
    setup_rust_trace,
)

MACRO_HOME = "/root/SmartC2Rust/macro"
TRANS_HOME = "/root/SmartC2Rust/trans"
C_PARSER_HOME = "/root/kiso-parser-c"
CONFIG_PATH = "/root/SmartC2Rust/config.json"

full_regions = []

DEBUG_LLM = False
TEST_MODE = None
FFI_STRATEGY = None

REPAIR_MAX = 500
given_time = 60 #200

iteration_dict = {}

####################################################
########## instrument_io
####################################################

def debug_with_pexpect(c_run_script, target_dir, breakpoints=None, use_breakpoints=True):
    """
    Interactively control rust-gdb using pexpect (no timeout).
    
    Args:
        c_run_script: Path to the executable to debug
        target_dir: Target directory
        breakpoints: List of breakpoints to set
        use_breakpoints: Whether to use breakpoints (True/False)
    """
    try:
        import pexpect
    except ImportError:
        print("Error: pexpect not installed. Install with: pip install pexpect")
        return False
    
    c_run_script = Path(c_run_script)
    
    if not c_run_script.exists():
        print(f"Error: Executable not found at {c_run_script}")
        return False
    
    if breakpoints is None:
        breakpoints = ["main"]
    
    print(f"\n=== Debugging {c_run_script} with pexpect-controlled rust-gdb ===")
    print(f"Use breakpoints: {use_breakpoints}\n")
    
    # Launch rust-gdb
    gdb = pexpect.spawn(f"rust-gdb -q {c_run_script}")
    gdb.logfile = open("/tmp/gdb_log.txt", "wb")
    
    try:
        # Wait for GDB prompt (no timeout)
        gdb.expect(r"\(gdb\)")
        
        # Configure settings
        gdb.sendline("set print pretty on")
        gdb.expect(r"\(gdb\)")
        
        # Set breakpoints (only when use_breakpoints is True)
        if use_breakpoints:
            for bp in breakpoints:
                print(f"Setting breakpoint at {bp}")
                gdb.sendline(f"break {bp}")
                gdb.expect(r"\(gdb\)")
                print(gdb.before.decode())
        
        # Run the program
        print("\nRunning program...")
        gdb.sendline("run")
        gdb.expect(r"\(gdb\)")
        print(gdb.before.decode())
        
        # Display backtrace
        print("\nBacktrace:")
        gdb.sendline("backtrace")
        gdb.expect(r"\(gdb\)")
        print(gdb.before.decode())
        
        if use_breakpoints:
            # Only execute the following when breakpoints are enabled
            
            # Display local variables
            print("\nLocal variables:")
            gdb.sendline("info locals")
            gdb.expect(r"\(gdb\)")
            print(gdb.before.decode())
            
            # Step to the next line
            print("\nNext line:")
            gdb.sendline("next")
            gdb.expect(r"\(gdb\)")
            print(gdb.before.decode())
            
            # Display variables
            print("\nPrinting variables:")
            gdb.sendline("info locals")
            gdb.expect(r"\(gdb\)")
            print(gdb.before.decode())
            
            # Continue execution
            print("\nContinuing...")
            gdb.sendline("continue")
            gdb.expect(r"\(gdb\)")
            print(gdb.before.decode())
            
            # Check Rust variables at the next breakpoint
            print("\nAt next breakpoint - checking Rust variables:")
            gdb.sendline("info args")
            gdb.expect(r"\(gdb\)")
            print(gdb.before.decode())
            
            gdb.sendline("info locals")
            gdb.expect(r"\(gdb\)")
            print(gdb.before.decode())
            
            # Run to completion (skip all remaining breakpoints)
            print("\nRunning to completion...")
            gdb.sendline("continue")
            gdb.expect(r"\(gdb\)")
            print(gdb.before.decode())
        
        # Quit GDB
        print("\nQuitting GDB...")
        gdb.sendline("quit")
        gdb.expect(pexpect.EOF)
        
        print("\n=== Debug session completed ===")
        
        return True
        
    except Exception as e:
        print(f"\nError during debugging: {e}")
        import traceback
        traceback.print_exc()
        return False
    finally:
        gdb.close()
        print("Full log saved to /tmp/gdb_log.txt")


def save_report_data(archive_dir, result_path, dep_json_path, meta_dir, target, exec_time):
    c_paths = []
    
    dep_json = read_json(dep_json_path)
    for item in dep_json:
        if 'div_parts' in item:
            for div in item['div_parts']:
                c_paths.append(div['source'])
        else:
            c_paths.append(item['source'])
    
    result = {}
    sum_file_total = 0
    sum_func_count = 0
    for file_path in c_paths:
        meta_data, meta_path = obtain_metadata(file_path, meta_dir, False, None, "def")
        
        func_count = 0
        file_total = 0
        for item in meta_data:
            if item['category'] == 'function' and 'equivalence' in item:
                file_total += item['equivalence']
                func_count += 1
            
            if 'components' in item:
                for com in item['components']:
                    if com['category'] == 'function' and 'equivalence' in com:
                        file_total += com['equivalence']
                        func_count += 1

        if func_count != 0:
            file_result = file_total / func_count
        else:
            file_result = 0
        file_result = round(file_result, 5)
        result[file_path] = file_result

        sum_file_total += file_total
        sum_func_count += func_count
    
    print("----------------------------------------------")
    print("Caluculate equivalence_rata...")
    print()

    print(f"File Result:")
    for file_path in result:
        print(f"   {result[file_path]} for {file_path}")
    print("----------------------------------------------")
    if sum_func_count != 0:
        sum_result = sum_file_total / sum_func_count
    else:
        sum_result = 0
    sum_result = round(sum_result, 5)
    print(f"Total: {sum_result} for {target}")

    result_json = read_json(result_path)

    # Inherit from moment_path
    moment_json = read_json(moment_path)

    if not os.path.exists(f"{database_dir}/s_repair_count.json"):
        write_json(f"{database_dir}/s_repair_count.json", {})

    f_count_json = read_json(f"{database_dir}/s_repair_count.json")
    if target not in f_count_json:
        f_count_json[target] = 0
    f_count = f_count_json[target]
    trial_id = "trial_" + str(f_count) 
    f_count_json[target] = f_count + 1
    write_json(f"{database_dir}/s_repair_count.json", f_count_json)

    if result_json is None:
        result_json = {}
    if target not in result_json:
        result_json[target] = {}
    if trial_id not in result_json[target]:
        result_json[target][trial_id] = {}
        
    destination = archive_dir + "/" + target + "/" + "s_repair_" + trial_id

    calculate_execution_time(chat_dir, "time.json", trial_id, target)

    copy_directory(exp_dir, destination)
    copy_directory(chat_dir, destination)

    copy_directory(mix_io_dir, destination)
    copy_file(log_file_path, destination)
    # copy_file(dep_json_path, destination)
    copy_file(token_path, destination)
    # copy_file("token_macro.json", destination)
    copy_file("time.json", destination)
    # copy_file("classify_data.json", destination)

    current_directory = os.getcwd()
    result_json[target][trial_id]['cwd'] = current_directory
    result_json[target][trial_id]['exec_time'] = exec_time
    result_json[target][trial_id]['archive_dir'] = destination
    result_json[target][trial_id]['average'] = None #moment_json[target][trial_id]['average']

    if trial_id not in result_json[target]:
        result_json[target][trial_id] = {}

    result_json[target][trial_id]['equivalence_average'] = None
    result_json[target][trial_id]['compile_average'] = None # Store the average compile count across all files

    if 'equivalence_details' not in result_json[target]:
        result_json[target][trial_id]['equivalence_details'] = {}

    if 'compile_details' not in result_json[target]:
        result_json[target][trial_id]['compile_details'] = {}

    if 'input_token' not in result_json[target]:
        result_json[target][trial_id]['input_token'] = {}
    
    if 'output_token' not in result_json[target]:
        result_json[target][trial_id]['output_token'] = {}
    
    result_json[target][trial_id]['equivalence_average'] = sum_result

    write_json(result_path, result_json)


    ######### updating result.json
    """
    result_json = read_json(result_path)
    moment_json = read_json(moment_path)
    trial_id = "trial_" + str(moment_json[target]['current_count'] - 1) # Same in moment_path

    average = moment_json[target][trial_id]['average']
    sum_repair_count = 0
    file_count = 0

    for rust_path in moment_json[target][trial_id]['paths']:
        exp_path = obtain_exp_path(rust_path, average)
        exp_data = read_json(exp_path)

        if rust_path == "modified_rust/src/urlparser/test_c/parts0_h.rs":
            print("-----")
            #"modified_rust/src/build.rs": 1,
            #"modified_rust/src/urlparser/url_h/parts0_h.rs": 2,
            #"modified_rust/src/urlparser/test_c/parts0_h.rs": 2

        repair_count = 0  # Or a default value
        sum_input = 0
        sum_output = 0

        #repair_count = exp_data['trials'][-1] # Last element # build.rs is not working well.
        if exp_data is not None and 'trials' in exp_data and exp_data['trials']:
            repair_count = exp_data['trials'][-1]['repair_count']
            print(repair_count)
            if repair_count is None:
                repair_count = 0
            sum_repair_count += repair_count

            for item in exp_data['trials']:
                sum_input += item['input_token']
                sum_output += item['output_token']
        
        else:
            print("Error: Invalid or empty data")

        result_json[target][trial_id]['compile_details'][rust_path] = repair_count
        result_json[target][trial_id]['input_token'][rust_path] = sum_input
        result_json[target][trial_id]['output_token'][rust_path] = sum_output

        file_count += 1

    print("-------")
    print(rust_path)
    print(result_json[target][trial_id]['input_token'][rust_path])
    print(exp_path)
    #sum_repair_count / file_count
    if file_count == 0:
        result_json[target][trial_id]['compile_average'] = sum_repair_count
    else:
        result_json[target][trial_id]['compile_average'] = sum_repair_count / file_count
    """
    write_json(result_path, result_json)


def add_line_numbers_custom(input_file, fixed_number):
    try:
        # Create a temporary file
        with tempfile.NamedTemporaryFile(mode='w+', delete=False, encoding='utf-8') as temp_file:
            with open(input_file, 'r', encoding='utf-8') as infile:
                # Read all lines and get the maximum indent level and line count
                lines = list(infile)
                if not lines:
                    #print(f"File {input_file} is empty.")
                    return
                max_line_num = len(lines)
                max_indent = max((len(line) - len(line.lstrip())) // 4 for line in lines)
                
                # Calculate the maximum digit count for line numbers and indent levels
                line_num_width = len(str(max_line_num))
                indent_width = len(str(max_indent))
                
                # Create the format string (fix the position of the colon)
                format_str = f"Line{{:{line_num_width}d}} [{{:{indent_width}d}}]: {{}}"
                
                # Process each line
                for line_number, line in enumerate(lines, start=fixed_number):
                    indent_level = (len(line) - len(line.lstrip())) // 4
                    numbered_line = format_str.format(line_number, indent_level, line)
                    temp_file.write(numbered_line)
                
        # Overwrite the original file with the contents of the temporary file
        os.replace(temp_file.name, input_file)
        #print(f"Wrote file with line numbers and indent levels to {input_file}.")

    except IOError as e:
        print(f"An error occurred: {e}")



def find_matching_path(workspace_dir, target_suffix):

    matching_paths = []
    matching_path = target_suffix
    for root, _, files in os.walk(workspace_dir):
        for file in files:
            full_path = os.path.join(root, file)
            if full_path.endswith(target_suffix):
                matching_paths.append(full_path)
                matching_path = full_path
                break
    
    return matching_path


def support_returns(file_path):
    output_list = []

    tmp_file = "tmp.c"
    copy_file(file_path, tmp_file)
    remove_comments_file(tmp_file)

    with open(tmp_file, 'r') as f:
        lines = f.readlines()

    for i in range(0, len(lines)): #for i in range(input_line - 1, func_data['def_end_line']):
        output_line = None
        return_var = None
        current_line = lines[i]
        
        # 1. Extract the part that does not include comments
        code_parts = []
        in_comment = False
        in_string = False
        string_char = None
        j = 0
    
        while j < len(current_line):
            if in_string:
                if current_line[j] == string_char and current_line[j-1] != '\\':
                    in_string = False
                code_parts.append(current_line[j])
            elif in_comment:
                if current_line[j:j+2] == '*/':
                    in_comment = False
                    j += 1
            else:
                if current_line[j:j+2] == '//':
                    break
                elif current_line[j:j+2] == '/*':
                    in_comment = True
                    j += 1
                elif current_line[j] in '"\'':
                    in_string = True
                    string_char = current_line[j]
                    code_parts.append(current_line[j])
                else:
                    code_parts.append(current_line[j])
            j += 1
        
        code_line = ''.join(code_parts).strip()
        
        # 2. Pattern-match actual return statements
        if code_line and 'return' in code_line:
            # Basic patterns of return statements: 
            # - Starts with return at the beginning of the line (excluding whitespace)
            # - There is a space or parenthesis after return
            if (code_line.lstrip().startswith('return') and 
                (len(code_line.lstrip()) == 6 or  # return alone
                code_line.lstrip()[6] in ' \t(')): # Space or parenthesis after return
                
                start_line = i + 1 #output_line = i + 1

                current_line = start_line - 1 # in the sense of index
                end_line = start_line - 1  # in the sense of index # set default value
                
                while current_line < len(lines):
                    if ';' in lines[current_line]:
                        end_line = current_line
                        break
                    current_line += 1
                
                end_line = end_line + 1 # in terms of line number

                if start_line != end_line:
                    entry = {
                        "file_path" : file_path,
                        "start_line" : start_line,
                        "end_line" : end_line
                    }
                    output_list.append(entry)

    delete_file(tmp_file)

    return output_list



def get_rust_lib(file_path):
    base_path = os.path.splitext(file_path)[0]

    lib_header_path = f"{base_path}_rust.h" # Add _rust.h and create new path

    return lib_header_path

def create_rustlib_header(target_funcs, target_dir, list_path, meta_dir, ommited_paths): #, lib_header_path):

    order = read_compile_order(list_path)

    for file_path in order:
        if file_path in ommited_paths:
            continue
        # create header file
        lib_header_path = get_rust_lib(file_path)
        delete_file(lib_header_path)

        filename = os.path.splitext(os.path.basename(file_path))[0]  # Filename without extension
        filename = filename.upper().replace('-', '_')  # Convert hyphens to underscores and uppercase

        content = []
        content.append(f"#ifndef {filename}_RUST_LIB_H")
        content.append(f"#define {filename}_RUST_LIB_H")

        meta_data, meta_path = get_metadata(file_path, meta_dir, None)
        for func_data in meta_data:
            if func_data['category'] != 'function':
                continue
            if not is_target_func(func_data, target_funcs):
                continue

            args = func_data['arguments']
            arg_names = [f"{arg['actual_type']} {arg['var_name']}" for arg in args]  #[f"{arg['base_type']} {arg['var_name']}" for arg in args] #[f"{arg['register_type']}, {arg['var_name']}" for arg in args] #arg_names = [f"\"{arg['register_type']}\", {arg['var_name']}" for arg in args] # arg_names = [f"{arg['var_name']}" for arg in args] #[f"{arg['var_type']}, {arg['var_name']}" for arg in args]
            arg_str = f"{', '.join(arg_names)}" if arg_names else ""
           
            return_type = func_data['return_value']['actual_type']  #func_data['return_value']['base_type']

            content.append(f"extern {return_type} {func_data['rust_function_name']}({arg_str});")

        content.append("#endif")
        
        final_content = '\n'.join(content) # Join the list with newlines into a single string
        final_content += '\n' # Add a trailing newline

        write_file(lib_header_path, final_content)

        # add a include statement for the file_path
        with open(file_path, 'r') as f: # Add an include statement to the source file
            source_content = f.read()
        
        relative_header_path = os.path.relpath(lib_header_path, os.path.dirname(file_path)) # Get the relative path to the header file
        include_statement = f'#include "{relative_header_path}"\n'
        
        modified_content = include_statement + source_content # Prepend the include statement to the original file content
        
        with open(file_path, 'w') as f: # Write the updated content back to the file
            f.write(modified_content)


def generate_zombi(target_funcs, target_dir, list_path, meta_dir, type_json_path):

    seen_files = set()

    order = read_compile_order(list_path)
    for file_path in order:
        print(f"generate_zombi for {file_path}...")
        meta_data, meta_path = get_metadata(file_path, meta_dir, None)

        with open(file_path, 'r') as f:
            lines = f.readlines()

        #print(f"file_path is {file_path}")
        for func_data in meta_data:
            if func_data['category'] != 'function':
                continue
            if not is_target_func(func_data, target_funcs):
                continue

            if 'rust_function_name' not in func_data:
                continue

            # call_line = input_line + 1
            #print(json.dumps(func_data, indent=4))
            end_line = func_data['def_end_line']
            func_name = func_data['name']

            # Start from the function definition line and search for { #input_line = func_data['input_line'] # item
            input_line = None
            prefix = ""  # Part before {
            suffix = ""  # Part after {

            print("Before searching the function start...")
            for i in range(func_data['def_start_line'] - 1,  len(lines)): # end_line):
                if '{' in lines[i]:
                    input_line = i #+ 1
                    parts = lines[i].split('{', 1)
                    prefix = parts[0] + '{ ' # Keep the part before {
                    if len(parts) > 1:
                        suffix = parts[1]  # Keep the remaining part including {
                    else:
                        suffix = ""
                    break

            if input_line is None:
                print(f"Could not find function body start for {func_data['name']}")
                return line, None, line, None, None, None
            
            file_info = f'"{func_data["def_file_path"]}", "{func_data["def_start_line"]}"'
            return_arg = func_data['return_value']
            args = func_data['arguments']
            
            arg_names = []
            for arg in args:
                if arg['var_type'] == "union": 
                    continue
                formatted_arg =  f"{arg['var_name']}"
                arg_names.append(formatted_arg)

            arg_str = f", {', '.join(arg_names)}" if arg_names else ""

            if input_line is not None:
                wrapped_input = f'{prefix}PRINT_INPUT_{len(arg_names)}ARG({file_info}, {func_data["name"]}{arg_str});'

            return_var = "result"

            arg_names = [f"{arg['var_name']}" for arg in args]  #[f"{arg['base_type']} {arg['var_name']}" for arg in args] #[f"{arg['register_type']}, {arg['var_name']}" for arg in args] #arg_names = [f"\"{arg['register_type']}\", {arg['var_name']}" for arg in args] # arg_names = [f"{arg['var_name']}" for arg in args] #[f"{arg['var_type']}, {arg['var_name']}" for arg in args]
            arg_str = f"{', '.join(arg_names)}" if arg_names else ""
            
            if return_arg['actual_type'] == "void":
                call_statement = f"{func_data['rust_function_name']}({arg_str});" # rust_{func_data['rust_function_name']}
                return_statement = ""
            else:
                call_statement = f"{return_arg['actual_type']} {return_var} = {func_data['rust_function_name']}({arg_str});" # rust_{func_data['rust_function_name']}
                return_statement = f"return {return_var};"
            
            is_void = return_arg['register_type'].strip() == 'void'
            output_file_info = f'"{file_path}", "{end_line - 1}"'  # Use the actual line number
            if is_void:
                #wrapped_output = f'{return_prefix}PRINT_OUTPUT_VOID({output_file_info}, {func_data["name"]}); {return_suffix}'
                wrapped_output = f'PRINT_OUTPUT_VOID({output_file_info}, {func_name});' # {return_suffix}'
            else:
                #wrapped_output = f'{return_prefix}PRINT_OUTPUT({output_file_info}, {func_data["name"]}, {return_var}); {return_suffix}'
                wrapped_output = f'return PRINT_OUTPUT({output_file_info}, {func_name}, {return_arg['base_type']}, {return_arg['register_type']}, {return_var});' #  {return_suffix}'
            
            total_statement = f"{prefix} {call_statement} {return_statement}\n"  #total_statement = f"{wrapped_input} {call_statement} {wrapped_output}\n" # {suffix}"
            
            
            lines[input_line] = total_statement # call_statement
            #lines[output_line] = return_statement

            # Count the original number of lines
            original_line_count = end_line - input_line - 1 # - 1 # This might be due to the .._rust.h insertion

            # Set the range based on the original line count
            for i in range(input_line + 1, input_line + 1 + original_line_count):
                original_line = lines[i]
                leading_spaces = len(original_line) - len(original_line.lstrip())
                indent = "    "
                lines[i] = f"{indent}// removed\n"
            
            # Add a blank line after the function
            #lines[input_line + 1 + original_line_count] = "\n"
            """
            if input_line < end_line - 1:
                # Fix the range: from the line after input_line to the line before end_line
                for i in range(input_line + 1 + 1, end_line):
                    original_line = lines[i]
                    leading_spaces = len(original_line) - len(original_line.lstrip())
                    indent = "    " #" " * leading_spaces
                    lines[i] = f"{indent}// removed\n"
            """

            if file_path not in seen_files:
                seen_files .add(file_path)
        with open(file_path, 'w') as f:
                f.writelines(lines)

    return list(seen_files)

         
def merge_metadata(list_path, io_list_path, meta_dir, o_meta_dir, c_io_dir, raw_dir):

    order = read_compile_order(list_path)  # This is the order.txt before merging
    print(f"order at merge_metadata(): {order}")
    # Updating metadata becomes recursive by the amount being updated. Only o_meta_dir should be updated
    
    """
    def_file_paths =[]
    for file_path in order:
        meta_data, meta_path = obtain_metadata(file_path, meta_dir, False, None, "def")
        for item in meta_data:
            print("Round 1")
            print(f"meta_path: {meta_path}")
            def_file_path = item['o_file_path']
            print(f"def_file_path: {def_file_path}")

            def_file_path = remove_base_path(def_file_path, raw_dir)
            def_file_path = f"{c_io_dir}/{def_file_path}"
            print(f"def_file_path: {def_file_path}")

            item['o_file_path'] = def_file_path

            def_file_paths.append(def_file_path)

        write_json(meta_path, meta_data)
    
    print("End of Round 1")
    #print(def_file_paths)
    for item in def_file_paths:
        print(item)
    """

    for file_path in order:
        meta_data, meta_path = obtain_metadata(file_path, meta_dir, False, None, "def")
        for item in meta_data:
            o_file_path = item['o_file_path']
            print(f"meta_path: {meta_path}")
            print(f"def_file_path: {o_file_path}")

            o_file_path = remove_base_path(o_file_path, raw_dir)
            o_file_path = f"{c_io_dir}/{o_file_path}"

            o_meta_data, o_meta_path = get_metadata(o_file_path, o_meta_dir, None)
            print(f"o_meta_path: {o_meta_path}")
            print(f"new def_file_path: {o_file_path}")

            if o_meta_data is None:
                continue

            for o_item in o_meta_data:
                if (o_item['def_file_path'] == o_file_path and
                    o_item['def_start_line'] == item['o_start_line'] and
                    o_item['def_end_line'] == item['o_end_line']):
                    
                    if 'rust_function_name' in item:
                        o_item['rust_function_name'] = item['rust_function_name']

            write_json(o_meta_path, o_meta_data)

    print(f"o_meta_dir is {o_meta_dir}")
    io_order = read_compile_order(io_list_path)  # This is the order.txt before merging

    print(f"io_order at merge_metadata(): {io_order}")

    for file_path in io_order:
        o_meta_data, o_meta_path = get_metadata(file_path, o_meta_dir, None)
        if o_meta_data is None: # There is a possibility that print_io.h is included
            continue

        found = False 
        for o_item in o_meta_data:

            raw_file_path = remove_base_path(file_path, c_io_dir)
            raw_file_path = f"{raw_dir}/{raw_file_path}"

            meta_data, meta_path = obtain_metadata(raw_file_path, meta_dir, False, None, "def")


            if meta_data is None:
                print(file_path)
                continue
            
            for item in meta_data:
                if 'rust_function_name' not in item:
                    continue
                    
                if (item['o_file_path'] == raw_file_path and
                   item['o_start_line'] == o_item['def_start_line'] and
                   item['o_end_line'] == o_item['def_end_line']):

                    o_item['rust_function_name'] = item['rust_function_name']

    write_json(o_meta_path, o_meta_data)


def instrument_io(target_funcs, o_dep_json_path, o_run_path, run_test_path, list_path, io_list_path, c_io_dir, o_meta_dir, c_flow_path, c_log_path, target, func_json_path, type_json_path, raw_dir, golden_flow_path, all_struct_path, all_typedef_path, ommited_files): #target_dir, list_path, meta_dir, main_dir):
    c_io_tmp = "c_io_tmp"
    c_io_print = "c_io_print"

    c_io_tmp = f"{mix_io_dir}/{c_io_tmp}"
    c_io_print = f"{mix_io_dir}/{c_io_print}"


    copied_dir = create_backup_directory(c_io_dir)
    rename_directory(copied_dir, c_io_tmp)

    plain_used_path = "used_plain.json"
    used_path = "used.json"
    o_compile_log_path = "c_io_comile.log"

    # Functions imported from io checker
    analyze_call_graph(o_dep_json_path, o_run_path, io_list_path, f"{c_io_dir}/{target}", o_meta_dir, target, func_json_path, type_json_path, all_struct_path, all_typedef_path, o_compile_log_path, ommited_files)
    
    analyze_type(f"{c_io_dir}/{target}", o_dep_json_path, io_list_path, o_meta_dir, type_json_path, all_struct_path, all_typedef_path, plain_used_path, used_path)

    prepare_print_io(f"{c_io_dir}/{target}", io_list_path, o_dep_json_path, o_meta_dir, o_run_path, True, c_log_path, type_json_path, used_path, plain_used_path, target)

    # C: get golden c flows # When to capture this (whether the paths can change or we don't want them to change)
    get_golden_flows(c_io_dir, o_run_path, run_test_path, c_flow_path, c_log_path, o_meta_dir, golden_flow_path)

    ############# From here on, insertion of Rust FFI functions  #############

    rename_directory(c_io_dir, c_io_print)
    rename_directory(c_io_tmp, c_io_dir)

    delete_directory(c_io_print)

    # Re-acquire metadata
    previous_ommited_files = ommited_files
    ommited_files = []  # Need to clear here
    analyze_call_graph(o_dep_json_path, o_run_path, io_list_path, f"{c_io_dir}/{target}", o_meta_dir, target, func_json_path, type_json_path, all_struct_path, all_typedef_path, o_compile_log_path, ommited_files) 
    
    analyze_type(f"{c_io_dir}/{target}", o_dep_json_path, io_list_path, o_meta_dir, type_json_path, all_struct_path, all_typedef_path, plain_used_path, used_path)

    # Merge metadata and original metadata (o_meta)
    merge_metadata(list_path, io_list_path, meta_dir, o_meta_dir, c_io_dir, raw_dir)

    # Prepare for inserting Rust functions
    create_rustlib_header(target_funcs, c_io_dir, io_list_path, o_meta_dir, previous_ommited_files)

    # Insert Rust functions
    log_files = generate_zombi(target_funcs, c_io_dir, io_list_path, o_meta_dir, type_json_path)

    # Process headers for PRINT_IO
    function_used = get_function_used(c_io_dir, io_list_path, o_meta_dir, type_json_path, plain_used_path)
    write_json("function_used.json", function_used)

    header_path = 'print_io.h'
    normal_member_counts = []
    struct_member_counts = []
    input_arg_counts = []
    fuzz_flag = False


def modify_build(mix_io_dir, c_io_dir, o_run_path, o_meta_dir, rust_io_dir, run_all_path, run_test_path):
    print("Modifying the build system...")
    exp_data = {}
    exp_data['experiment_path'] = "build_system.c"
    exp_data['file_path'] = "build_system.c"
    exp_data['repair_count'] = 0
    exp_data['average'] = 0

    create_permissioned_file(run_all_path)

    repair_count = 1
    interface = {
        "mix_io_dir" : mix_io_dir,
        "c_io_dir" : c_io_dir,
        "o_run_path" : o_run_path,
        "meta_dir" : o_meta_dir,
        "exp_data" : exp_data,
        "rust_io_dir" : rust_io_dir,
        "repair_count" : repair_count,
        "run_all_path" : run_all_path,
        "run_test_path" : run_test_path
    }
    repair_execute("modify_link", interface)


def modify_test_run(mix_io_dir, c_io_dir, o_run_path, o_meta_dir, rust_io_dir, run_all_path, run_test_path):

    exp_data = {}
    exp_data['experiment_path'] = "test_run.c"
    exp_data['file_path'] = "test_run.c"
    exp_data['repair_count'] = 0
    exp_data['average'] = 0

    create_permissioned_file(run_all_path)

    repair_count = 1
    interface = {
        "mix_io_dir" : mix_io_dir,
        "c_io_dir" : c_io_dir,
        "o_run_path" : o_run_path,
        "meta_dir" : o_meta_dir,
        "exp_data" : exp_data,
        "rust_io_dir" : rust_io_dir,
        "repair_count" : repair_count,
        "run_all_path" : run_all_path,
        "run_test_path" : run_test_path
    }
    repair_execute("modify_test", interface)



solo_id = 0

# Since this uses append_file, need to determine where to clear it
def filter_cloned_target_logs(log_path, log_out_path):
    log_content = read_file(log_path)
    global solo_id
    # Split each line and filter only lines starting with 'cloned_target'
    filtered_lines = [f"Fuzzing ID: solo_{solo_id}"]
    filtered_lines.extend([
        line for line in log_content.split('\n')
        if line.strip().startswith('cloned_target')  #if line.strip().startswith('TRACK cloned_target')
    ])
    
    # Join the filtered lines
    content = '\n'.join(filtered_lines)
    append_file(log_out_path, content)
    #return '\n'.join(filtered_lines)

    solo_id += 1


def rust_parse_struct_value(struct_str):
    # Separate the struct type and parameters
    match = re.match(r'(\w+)\((.*)\)', struct_str)
    if not match:
        return struct_str.strip()
    
    struct_type = match.group(1)
    params = match.group(2)
    result = {}
    
    # Parse parameters
    param_list = []
    current_param = ''
    bracket_count = 0
    
    for char in params:
        if char == '(' or char == '{':
            bracket_count += 1
            current_param += char
        elif char == ')' or char == '}':
            bracket_count -= 1
            current_param += char
        elif char == ',' and bracket_count == 0:
            param_list.append(current_param.strip())
            current_param = ''
        else:
            current_param += char
    
    if current_param:
        param_list.append(current_param.strip())
    
    # Parse each parameter
    for param in param_list:
        if '=' in param:
            name, value = param.split('=', 1)
            name = name.strip()
            value = value.strip()
            # Recursively parse inner structs
            if '(' in value or '{' in value:
                result[name] = rust_parse_struct_value(value)
            else:
                result[name] = value
    
    return result


def get_rust_arguments(line_idx, lines):
    args_dict = {}
    if line_idx + 1 < len(lines):
        next_line = lines[line_idx + 1]
        
        # Extract Enter_log values
        if "Enter_log:::" in next_line:
            # Split by "Enter_log:::" and get the part after it
            value_part = next_line.split("Enter_log:::")[-1]
            # Extract variable name and value
            if "=" in value_part:
                var_name, var_value = value_part.split("=", 1)
                args_dict[var_name.strip()] = var_value.strip()
    
    return args_dict 

def get_rust_returns(line_idx, lines):
    args_dict = {}
    if line_idx - 1 >= 0:  # Check the line before the exit log
        return_line = lines[line_idx - 1]
        
        if "Return_log:::" in return_line:
            value_part = return_line.split("Return_log:::")[-1]
            if "=" in value_part:
                var_name, var_value = value_part.split("=", 1)
                args_dict[var_name.strip()] = var_value.strip()
    
    return args_dict

def rust_parse_log(log_file_path, rust_flow_path):
    log_content = read_file(log_file_path)

    if log_content is None:
        print("Not found rust frow log")
        return

    lines = log_content.split('\n')

    results = []
    call_stack = {}
    
    for line_idx, line in enumerate(lines): 
        log_line = line_idx + 1
        if not line.strip():
            continue
            
        if ':::' in line:
            # Get the part after the timestamp and log level
            log_parts = line.split(' INFO ')[-1]
            
            # Get the part starting from the last 7GqX9B2:::
            function_blocks = log_parts.split('7GqX9B2:::')
            last_block = function_blocks[-1] if function_blocks else ''
            
            if last_block:
                # Extract information from the last block
                parts = last_block.split(':::')
                if len(parts) >= 3:
                    function_name = parts[0]
                    file_path = parts[1]
                    file_line = parts[2].split(':')[0]
                    
                    """
                    # Extract arguments - search for {} within the last block
                    args_dict = {}
                    if '{' in last_block and '}' in last_block:
                        args_str = last_block[last_block.find('{')+1:last_block.find('}')]
                        if args_str:
                            # Temporarily replace values containing spaces before splitting by spaces
                            temp_args_str = args_str
                            quoted_values = []
                            import re
                            
                            # Find quoted values and replace with temporary placeholders
                            quoted_pattern = r'"[^"]*"'
                            for idx, match in enumerate(re.finditer(quoted_pattern, args_str)):
                                placeholder = f"__QUOTED_{idx}__"
                                quoted_values.append(match.group())
                                temp_args_str = temp_args_str.replace(match.group(), placeholder)
                            
                            # Split by spaces and process
                            arg_pairs = temp_args_str.split()
                            for pair in arg_pairs:
                                if '=' in pair:
                                    name, value = pair.split('=', 1)
                                    # Restore placeholders to original values
                                    for idx, quoted_value in enumerate(quoted_values):
                                        value = value.replace(f"__QUOTED_{idx}__", quoted_value)
                                    args_dict[name] = value
                    """

                    if ": enter" in line:
                        args_dict = get_rust_arguments(line_idx, lines)

                        current_call = {
                            "name": function_name,
                            #"function_name": function_name,
                            "file_path": file_path,
                            #"line_number": file_line,
                            "def_start_line": file_line,
                            "log_line" : log_line,
                            "call_type": "input",
                            "arguments": args_dict
                        }
                        call_stack[function_name] = current_call
                        results.append(current_call)
                        
                    elif ": exit" in line:
                        args_dict = get_rust_returns(line_idx, lines)

                        output_call = {
                            "name": function_name,
                            "file_path": file_path,
                            #"line_number": file_line,
                            "def_start_line": file_line,
                            "log_line" : log_line,
                            "call_type": "output",
                            "return_value": args_dict
                        }
                        results.append(output_call)
    
    write_json(rust_flow_path, results)

    moment_rust_flows = read_json(rust_flow_path)
    for item in moment_rust_flows:
        if 'arguments' in item:
            all_values = []
            values = list(item['arguments'].values())
            # Add to the list
            all_values.extend(values)
            item['arg_list'] = all_values

    write_json(rust_flow_path, moment_rust_flows)
    moment_rust_flows = read_json(rust_flow_path)

    # Find the boundaries between test cases
    flow_sum_path = "flow_sum.json"
    find_test_lines(rust_log_path, flow_sum_path) 

    insert_test_lines(rust_flow_path, flow_sum_path)

    add_depth_to_calls(rust_flow_path)

    return results


def parse_arguments(input_str: str) -> Dict[str, str]:  # Parse arguments from input string into a dictionary with named arguments

    if not input_str or input_str == "()":
        return {}
    
    # Remove outer parentheses
    input_str = input_str.strip('()')
    
    args_dict = {}
    # Pattern 1: Extract argument names from print_input format
    # e.g.: (int, 5, char*, "hello") -> {"int": "5", "char*": "hello"}
    pattern1 = re.findall(r'(\w+(?:\s*\*)*)\s*,\s*([^,]+)(?=\s*,\s*\w+(?:\s*\*)*|$)', input_str)
    if pattern1:
        for arg_type, arg_value in pattern1[::2]:  # Skip every other pair as they're types
            args_dict[f"arg{len(args_dict)+1}"] = arg_value.strip()
        return args_dict

    # Pattern 2: Struct format
    # e.g.: {x:1, y:2} -> {"x": "1", "y": "2"}
    if input_str.startswith('{'):
        input_str = input_str.strip('{}')
        pairs = re.findall(r'(\w+)\s*:\s*([^,}]+)', input_str)
        return {k: v.strip() for k, v in pairs}
    
    # Pattern 3: Comma-separated values only
    # e.g.: 5, "hello", true -> {"arg1": "5", "arg2": "hello", "arg3": "true"}
    values = [v.strip() for v in input_str.split(',') if v.strip()]
    return {f"arg{i+1}": v for i, v in enumerate(values)}



def c_parse_log_2(log_path: str, c_flow_path: str) -> Dict:
    log_text = read_file(log_path)
    
    def parse_function_call(line, log_line) -> Optional[Dict]:  # Parse a single function call line into structured data with named arguments
        # Get execution order number
        order_match = re.match(r'<(\d+)>\s+(.+)', line)
        if not order_match:
            return None
        
        order_num = int(order_match.group(1))
        line = order_match.group(2)
        
        # Parse basic information
        basic_match = re.match(r'([^:]+):\s*(\d+):\s*([^:]+):', line)
        if not basic_match:
            return None
        
        file_path, line_number, function_name = basic_match.groups()
        
        found_data = {
            "oder" : order_num,
            "name": function_name.strip(),
            "def_file_path": file_path.strip(),
            "line_number": int(line_number),
            "call_type": None,  # Align with Rust log format
            "function": function_name.strip()  # Keep function name duplicated (to align with Rust format)
        }
        
        # Extract input arguments and convert to named arguments
        input_match = re.search(r'input:\s*(.*?)(?=\s*output:|$)', line)
        if input_match:
            input_str = input_match.group(1).strip()
            found_data["arguments"] = parse_arguments(input_str)
            found_data["call_type"] = "input"
        else:
            found_data["arguments"] = {}
        
        # Extract output value
        output_match = re.search(r'output:\s*(.*?)$', line)
        if output_match:
            output_str = output_match.group(1).strip()
            if output_str == "null":
                found_data["return_value"] = None
            else:
                found_data["return_value"] = output_str

            found_data["call_type"] = "output"
        
        return found_data

    function_calls = []
    
    for line in log_text.split('\n'):
        line = line.strip()
        if not line or line.startswith("Fuzzing ID:"):
            continue
        
        parsed_data = parse_function_call(line, log_line)
        if parsed_data:
            function_calls.append(parsed_data)
    
    # Sort by execution order (optional)
    function_calls.sort(key=lambda x: x["line_number"])
    write_json(c_flow_path, function_calls)

    return function_calls



def extract_struct_values(struct_str):
    """
    Extract values from a structure string like {name:John, age:30}
    Returns the values only, without the keys
    """
    # Remove curly braces
    content = struct_str.strip('{}')
    if not content:
        return []
    
    # Split key-value pairs
    pairs = content.split(',')
    values = []
    for pair in pairs:
        if ':' in pair:
            # Get only the value part after the colon
            value = pair.split(':')[1].strip()
            values.append(value)
    return values


def parse_input(input_part):
    """
    Extract input values from log string, including values inside structures.
    
    Args:
        input_str (str): Input string containing "input: (...)" format
        
    Returns:
        list: List of extracted values
    
    Example:
        >>> extract_input_values('input: (123, {name:John, age:30})')
        ['123', 'John', '30']
    """

    # Get the contents inside parentheses
    if '(' not in input_part or ')' not in input_part:
        return []
    
    bracket_content = input_part[input_part.find('(') + 1:input_part.find(')')]
    
    # Return empty list for empty parentheses
    if not bracket_content.strip():
        return []
    
    # Split by comma and get each value
    values = []
    current_value = ''
    in_quotes = False
    brace_count = 0
    
    for char in bracket_content:
        if char == '"' or char == "'":
            in_quotes = not in_quotes
            current_value += char
        elif char == '{':
            brace_count += 1
            current_value += char
        elif char == '}':
            brace_count -= 1
            current_value += char
        elif char == ',' and not in_quotes and brace_count == 0:
            values.append(current_value.strip())
            current_value = ''
        else:
            current_value += char
    
    if current_value:
        values.append(current_value.strip())
    
    # List to store the final values
    final_values = []
    
    # Process each value
    for value in values:
        # If it's a struct
        if value.startswith('{') and value.endswith('}'):
            struct_values = extract_struct_values(value)
            final_values.extend(struct_values)
        else:
            # Remove quotes for strings
            if (value.startswith('"') and value.endswith('"')) or \
               (value.startswith("'") and value.endswith("'")):
                value = value[1:-1]
            final_values.append(value)
    
    return final_values


def get_var_name_list(meta_dir, file_path, def_start_line, name):
    var_name_list = []
    meta_data, meta_path = get_metadata(file_path, meta_dir, None)

    print(meta_path)
    for item in meta_data:
        if def_start_line == item['def_start_line'] and name == item['name']:
            if 'arguments' not in item:
                continue
            arguments = item['arguments']
            for arg in arguments:
                var_name_list.append(arg['var_name'])

    return var_name_list


def check_script_state(script_path: str, timeout: int = 5) -> Tuple[bool, str]:
    absolute_path = os.path.abspath(script_path)
    print(f"Absolute path of script: {absolute_path}")

    if not os.path.exists(absolute_path):
        return False, f"Script does not exist: {absolute_path}"

    if not os.access(absolute_path, os.X_OK):
        return False, f"Script does not have execute permission: {absolute_path}"

    try:
        # Execute the script
        process = subprocess.Popen(
            [absolute_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            stdin=subprocess.PIPE,
            # Execute through the shell
            shell=True
        )

        # Monitor the process state
        psutil_process = psutil.Process(process.pid)
        start_time = time.time()

        while time.time() - start_time < timeout:
            # Check the process state
            if process.poll() is not None:
                stdout, stderr = process.communicate()
                return False, f"Script has terminated. Exit code: {process.returncode}"

            # Get the process state (output more detailed information)
            status = psutil_process.status()
            cpu_percent = psutil_process.cpu_percent(interval=0.1)
            #print(f"Status: {status}, CPU: {cpu_percent}%")  # Debug output

            # Check characteristics of waiting-for-input state
            if (status == 'sleeping' and 
                cpu_percent < 0.1):
                
                # Terminate the process
                process.terminate()
                return True, "Likely in a waiting-for-arguments state"

            time.sleep(0.1)

        # If timed out
        process.terminate()
        return False, "Processing in progress or unknown state (timed out)"

    except Exception as e:
        return False, f"An error occurred: {str(e)}"



#sudo ln -s $(pwd)/shell_runner/target/release/shell_runner /usr/local/bin/
def run_script_flow(script_path, timeout, dir_move_flag, execute_log_path, option, rust_log_path, golden_flow_path): # -> Union[str, None]:
    
    error_output = None
    std_output = None
    try:
        # Check if file exists
        if not os.path.exists(script_path):
            raise FileNotFoundError(f"Script not found: {script_path}")
            
        # Check if file is executable
        if not os.access(script_path, os.X_OK):
            # Try to make it executable
            try:
                os.chmod(script_path, 0o755)
            except Exception as e:
                raise PermissionError(f"Cannot make script executable: {e}")
        
        if dir_move_flag is True:
            execute_dir = os.path.dirname(os.path.normpath(script_path))
            script_path = os.path.basename(os.path.normpath(script_path))
        else:
            execute_dir = None

        """
        # Check permission information and output detailed logs
        file_stat = os.stat(script_path)
        current_user = os.getlogin()
        
        try:
            owner = pwd.getpwuid(file_stat.st_uid).pw_name
            group = grp.getgrgid(file_stat.st_gid).gr_name
        except KeyError:
            owner = str(file_stat.st_uid)
            group = str(file_stat.st_gid)
            
        print(f"File permissions: {oct(file_stat.st_mode)}")
        print(f"File owner: {owner}, group: {group}")
        print(f"Current user: {current_user}")

        # Check and set execute permission
        if not os.access(script_path, os.X_OK):
            print("Script is not executable. Attempting to set execute permission...")
            
            try:
                # Change permissions using sudo (if necessary)
                if owner != current_user:
                    result = subprocess.run(['sudo', 'chmod', '+x', script_path], 
                                            capture_output=True, text=True)
                    if result.returncode != 0:
                        print(f"sudo chmod failed: {result.stderr}")
                        # If sudo fails, try regular chmod
                        os.chmod(script_path, 
                                file_stat.st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)
                else:
                    # If the owner is the current user, chmod directly
                    os.chmod(script_path, 
                            file_stat.st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)
                
                print("Execute permission set successfully")
            except Exception as e:
                print(f"Failed to set execute permission: {e}")
                # Even if permission setting fails, still attempt to execute the script
        """

        """
        if option == "compile":
            script_path = f"./{script_path} --build"
        elif option == "run":
            script_path = f"./{script_path} --run"
        else:
            script_path = f"./{script_path}"

        """
        cmd = ["bash"]
        
        if script_path.startswith("./"):
            cmd.append(script_path)
        else:
            cmd.append(f"./{script_path}")

        if option == "compile":
            cmd.append("--build")
        elif option == "run":
            cmd.append("--run")
        
        print(f"Execute run_script: {script_path} at {execute_dir}")

        # Execute the script
        if execute_dir is None:
            print("Run with None execute_dir")
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=timeout,
                shell=False #shell=True  # Required for shell scripts
            )
        else:
            result = subprocess.run(
                cmd,
                capture_output=True,
                cwd=execute_dir,
                text=True,
                timeout=timeout,
                shell=False #shell=True  # Required for shell scripts
            )

        # Write execution results to the log
        if execute_log_path is not None:
            print(f"before ({execute_log_path})")
            create_file(execute_log_path)
            with open(execute_log_path, 'w', encoding='utf-8') as f:
                if result.stdout:
                    f.write(result.stdout)
                    #print(result.stdout)
                if result.stderr:
                    f.write(result.stderr)
                    #print(result.stderr)
            print(f"Wrote log file ({execute_log_path})")

        # Check for errors
        std_output = result.stdout if result.stdout is not None and result.stdout != "" else None
        error_output = result.stderr if result.stderr is not None and result.stderr != "" else None
        return_code = result.returncode  # Get return_code here

        print(f"type(result.stdout): {type(result.stdout)}") 
        print(f"type(result.stderr): {type(result.stderr)}") 

        if process_type == "explore":
            moment_flow_path = 'flow_moment.txt'
            std_output = separate_output_files(std_output, moment_flow_path)

        
        if error_output and return_code == 0:
            if std_output is None:
                std_output = error_output
            else:
                std_output += "\n" + error_output  # Append with a newline
            error_output = None
        
        if error_output is None and return_code == 1: # Mainly for "translation_testcase"
            error_output = "Return code is 1, indicating abnormal termination."
        
        return error_output, std_output

    except subprocess.TimeoutExpired:
        return f"Script execution of {script_path} timed out after {timeout} seconds", std_output
        
    except subprocess.SubprocessError as e:
        return f"Failed to execute script: {str(e)}", std_output
        
    except Exception as e:
        return f"Unexpected error: {str(e)}", std_output


def get_golden_only(c_log_path, c_flow_path, golden_flow_path, o_meta_dir):
    c_parse_log(c_log_path, c_flow_path, o_meta_dir, False) 

    copy_file(c_flow_path, golden_flow_path)

    golden_flow, file_data = show_flow(golden_flow_path)
    os.makedirs('golden', exist_ok=True)
    for test_name, line_data in file_data.items():
        with open(f"golden/{test_name}.txt", 'w', encoding='utf-8') as f:
            for line in line_data:
                f.write(line + '\n')

    print(f"Saved golden flows at {golden_flow_path}")


def get_golden_flows(c_io_dir, o_run_path, run_test_path, c_flow_path, c_log_path, o_meta_dir, golden_flow_path):

    delete_file(c_flow_path)
    delete_file(c_log_path)
    error, std_out = run_script(o_run_path, given_time, True, None, "both")

    error, std_out = run_script(run_test_path, given_time, True, None, "both")
    test_run_dir = os.path.dirname(os.path.normpath(run_test_path))

    # This was written with absolute paths, so it's no longer needed
    obtained_c_log_path = f"{test_run_dir}/{c_log_path}"

    # Get and display data related to the adjust_opacity function
    solo_path = None # Not sure what this is
    if solo_path is not None:
        filter_cloned_target_logs(solo_path, c_log_path)
    
    print(f"o_run_path: {o_run_path}")
    print(f"run_test_path: {run_test_path}")
    c_parse_log(c_log_path, c_flow_path, o_meta_dir, True)

    copy_file(c_flow_path, golden_flow_path)

    golden_flow, file_data = show_flow(golden_flow_path)
    os.makedirs('golden', exist_ok=True)
    for test_name, line_data in file_data.items():
        with open(f"golden/{test_name}.txt", 'w', encoding='utf-8') as f:
            for line in line_data:
                f.write(line + '\n')

    print(f"Saved golden flows at {golden_flow_path}")


def obtain_impl_name(file_path, method_name, method_start_line):
    implementations = {}
    methods = []

    # Read the file content
    with open(file_path, 'r') as file:
        for line in file:
            parts = line.strip().split()
            if len(parts) < 4:
                continue
            kind = parts[1]
            element_name = parts[0]
            start_line = int(parts[2])
        
            if kind == 'implementation':
                implementations[start_line] = element_name

            elif kind == 'method':
                method_start_line = start_line
                methods.append((element_name, method_start_line))
        
    # Extract the implementation name for the given method
    impl_name = None
    impl_lines = sorted(implementations.keys())

    for impl_line in reversed(impl_lines):
        if method_start_line > impl_line:
            impl_name = implementations[impl_line]
            break
    
    return impl_name


def rust_find_struct_end(file_path, start_line):
    rust_code = read_file(file_path)

    struct_bounds = []
    lines = rust_code.split('\n')
    struct_start_stack = []
    outermost_struct_start = None

    # Regex to match struct start and end lines
    struct_start_pattern = re.compile(r'^\s*(pub\s+)?(struct|enum|type)\s+\w+\s*(\{|\()')
    struct_end_pattern = re.compile(r'^\s*\}')
    
    for i, line in enumerate(lines):
        if struct_start_pattern.match(line):
            struct_start_stack.append(i + 1)  # Line numbers are 1-based
            if outermost_struct_start is None:
                outermost_struct_start = i + 1
        elif struct_end_pattern.match(line) and struct_start_stack:
            struct_start_stack.pop()
            if not struct_start_stack and outermost_struct_start is not None:
                struct_bounds.append((outermost_struct_start, i + 1))
                outermost_struct_start = None
    
    for start, end in struct_bounds:
        if start == start_line:
            return end
    return None


def obtain_rust_interface(rust_source_file):
    tags_file = "rust_tags"
    delete_file(tags_file)

    command = f"ctags --languages=Rust --rust-kinds=fn -x -f {tags_file} {rust_source_file}"
    # command = f"ctags --languages=Rust --rust-kinds=fn -x -f {tags_file} {rust_source_file}"
    # command = f"ctags --fields=+n -x --c-kinds=+stfp -o {tags_file} {source_file}" # "--c-kinds=+st"
    # command = f"ctags --languages=Rust --rust-kinds=fn -x -o {tags_file} {rust_source_file}"
    # command = f"ctags --languages=Rust --rust-kinds=fn -f - --format=2 --excmd=number --fields=+n -o {tags_file} {rust_source_file}"
    subprocess.run(command, shell=True, capture_output=True, text=True)
    
    ctags_output = read_file(tags_file)
    functions = {}

    if ctags_output is None:
        return functions

    for line in ctags_output.split('\n'): 
        parts = line.split()  # Using split() without arguments to split by any whitespace
        if len(parts) >= 4:  # Ensure it's a function tag with sufficient parts
            func_name = parts[0]
            file_name = parts[1]
            line_number = parts[2]
            func_signature = ' '.join(parts[4:])  # Reconstruct the function signature
            func_signature = re.sub(r'\s*\{', '', func_signature)
            functions[func_name] = func_signature
                        
    delete_file(tags_file)

    return functions


def extract_function_arguments(file_path, start_line):
    """Extract function arguments from a Rust function definition."""
    with open(file_path, 'r') as file:
        lines = file.readlines()
        
    # Get the function definition line
    func_line = lines[start_line - 1].strip()
    
    # Find the opening parenthesis
    start_paren = func_line.find('(')
    if start_paren == -1:
        return []
        
    # Handle multi-line function signatures
    if ')' not in func_line:
        end_line = start_line
        while end_line < len(lines) and ')' not in lines[end_line - 1]:
            end_line += 1
        func_def = ''.join(lines[start_line - 1:end_line]).strip()
    else:
        func_def = func_line
    
    # Extract the content between parentheses
    try:
        args_str = func_def[func_def.find('(') + 1:func_def.find(')')].strip()
    except ValueError:
        return []
    
    if not args_str:
        return []
    
    # Parse the arguments
    args = []
    for arg in args_str.split(','):
        arg = arg.strip()
        if arg:
            # Extract just the parameter name (before the colon)
            param_name = arg.split(':')[0].strip()
            if param_name != 'self' and param_name != '&self':  # Exclude self parameter
                args.append(param_name)
    
    return args


def rust_find_variable_end(file_path: str, start_line: int) -> int:
    with open(file_path, 'r') as file:
        lines = file.readlines()
    
    brace_count = 0
    in_block = False
    
    for i, line in enumerate(lines[start_line - 1:], start=start_line):
        stripped_line = line.strip()
        
        # Detect the start of a block
        if '{' in stripped_line:
            in_block = True
            brace_count += stripped_line.count('{')
        
        # Detect the end of a block
        if '}' in stripped_line:
            brace_count -= stripped_line.count('}')
        
        # Detect the end of a variable definition
        if not in_block and (stripped_line.endswith(';') or stripped_line.endswith('}')):
            return i
        
        # Detect the end of a block
        if in_block and brace_count == 0:
            return i
    
    # If the end line is not found, return the last line of the file
    return len(lines)


def rust_parse_functions(rust_path, meta_dir):
    
    print(f"************** start parse_files for rust at {rust_path} **************")

    meta_path_rust = get_metadata(rust_path, meta_dir, True)

    tags_file = 'tags_output.txt'
    delete_file(tags_file)

    # ctags --fields=+n -x --c-kinds=+stfp -o tags_output.txt -R rust_sampmle/src/calculator.rs --languages=Rust
    command = f"ctags --fields=+n -x --c-kinds=+stfp -o {tags_file} -R {rust_path} --languages=Rust" # "--c-kinds=+st"
    print(command)
    subprocess.run(command, shell=True, capture_output=True, text=True)

    categories = {}  # Initialize a dictionary to hold categories and their elements
    meta_data = []
    with open(rust_path, 'r') as file:
        file_lines = file.readlines()
    
    impl_name = None
    arguments = []
    if not os.path.exists(tags_file):
        return

    with open(tags_file, 'r') as file:
        for line in file:
            parts = line.strip().split()
            if len(parts) < 4:
                continue
            kind = parts[1]
            element_name = parts[0]
            """
            if name == "main": # except main function
                continue
            """
            start_line = int(parts[2])
            context = parts[4]

            category = None
            end_line = None # initial value
            if kind == 'macro':
                category = 'macro_func'

            elif kind in ['static', 'constant', 'variable']:
                category = 'global_var'
                if kind == 'variable':
                    end_line = rust_find_variable_end(rust_path, start_line)
                else: 
                    end_line = rust_find_global_var_end(rust_path, start_line)

            elif kind in ['struct', 'enum', 'type']:
                category = 'data_type'
                end_line = rust_find_struct_end(rust_path, start_line)

            elif kind == 'function':
                category = 'function' 
                arguments = extract_function_arguments(rust_path, start_line)

            elif kind == 'method':
                category = 'function'
                impl_name = obtain_impl_name(tags_file, element_name, start_line)
                arguments = extract_function_arguments(rust_path, start_line)
            
            if category is None:
                continue

            if category not in categories:
                categories[category] = []
            
            if category is not None:

                item_data = {
                    "category": category,
                    "name": element_name,
                    "file_path": rust_path,
                    "start_line": start_line,
                    "end_line": end_line,
                }
                if category == "function":
                    item_data['arguments'] = arguments
            
                if impl_name is not None:
                    item_data['impl_name'] = impl_name
                
                meta_data.append(item_data)

            
    delete_file(tags_file)
    
    rust_interface_list = {}
    rust_interface_list = obtain_rust_interface(rust_path) # Actually, it might be okay to unify tags_file here

    # Get the directory path of the output file
    output_dir = os.path.dirname(meta_path_rust)

    # Create the directory if it does not exist
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    write_json(meta_path_rust, meta_data)

    print("************** end parse_files for rust **************")




def create_trace_macros(project_path):
    """
    Create a trace_macros.rs file that defines trace macros
    
    Args:
        project_path (str): Root path of the Rust project
    """
    # Create src directory if it does not exist
    target = os.path.join(project_path, 'src')
    os.makedirs(target, exist_ok=True)
    
    macro_content = '''#[macro_export]
macro_rules! trace {
    ($file:expr, $line:expr, $fn_name:expr, $($arg:expr),*) => {
        println!(concat!("ENTER [{}, line:{}] {} with args: "), $file, $line, $fn_name);
        $(
            print!("{:?} ", $arg);
        )*
        println!();
    };
}

#[macro_export]
macro_rules! trace_return {
    ($file:expr, $line:expr, $fn_name:expr, $ret:expr) => {
        println!(concat!("EXIT  [{}, line:{}] {} returned: {:?}"), $file, $line, $fn_name, $ret);
        $ret
    };
}'''
    
    # Write to trace_macros.rs file
    macro_file_path = os.path.join(target, 'trace_macros.rs')
    with open(macro_file_path, 'w') as f:
        f.write(macro_content)
    
    print(f"Created trace macros at: {macro_file_path}")


def is_target_func(item, target_funcs):
    key = f"{item['name']}"
    if key in target_funcs:
        return True
    return False


# // Changed to record all span events

initialize_tracing_fn = '''use std::fs::File;
use std::fs::OpenOptions;

pub fn initialize_tracing() {
    let file = OpenOptions::new()
        .create(true)
        .append(true)
        .open("/home/ubuntu/portable/out_flow_rust.log")
        .unwrap();
    
    tracing_subscriber::fmt()
        .with_ansi(false)
        .with_max_level(tracing::Level::DEBUG)
        .with_file(true)
        .with_line_number(true)
        .with_level(true)
        .with_span_events(tracing_subscriber::fmt::format::FmtSpan::NEW | 
                         tracing_subscriber::fmt::format::FmtSpan::ENTER | 
                         tracing_subscriber::fmt::format::FmtSpan::EXIT |
                         tracing_subscriber::fmt::format::FmtSpan::CLOSE)
        .with_writer(file)
        .init();
}
'''

def insert_trace_macros(target_funcs, file_path, meta_dir):
    with open(file_path, 'r') as f:
        lines = f.readlines()
        
    modified_lines = lines.copy()
        
    # Add use statement at the beginning
    use_statement = "use tracing::instrument;\n"
    modified_lines.insert(0, use_statement)
        
    relative_path = file_path.split('/')[-1]
    meta_data, meta_path = get_metadata(file_path, meta_dir, None)
    
    if meta_data is None:
        return

    # Sort in descending order by start_line
    sorted_meta_data = sorted(meta_data, key=lambda x: x['start_line'], reverse=True)

    # Process from the back
    for item in sorted_meta_data:
        if item['category'] == "function":
            name = item['name']
            start_line = item['start_line']
            rust_path = item['file_path']
                    
            # Get indentation of the function line
            function_line = modified_lines[start_line]
            indent = len(function_line) - len(function_line.lstrip())
                    
            # Analyze function definition line to determine modifiers
            is_pub = 'pub' in function_line
            is_extern_c = 'extern "C"' in function_line
                    
            # Generate trace and no_mangle macros on separate lines
            #trace_macro = " " * indent + "#[trace_all]\n"
            #trace_macro = " " * indent + f"#[instrument(name = \"7GqX9B2:::{name}:::{rust_path}:::{start_line}:::\", ret)]\n"
            trace_macro = " " * indent + f"#[instrument(name = \"7GqX9B2:::{name}:::{rust_path}:::{start_line}:::\", skip_all)]\n"
            no_mangle_macro = " " * indent + "#[no_mangle]\n" # "#[no_mangle]\n"
            
            if FFI_STRATEGY == "preserve":
                if name == 'main':
                    function_line = function_line.replace('fn main', 'fn rust_main')

            elif is_pub:
                print("May skipping")
                if FFI_STRATEGY == "preserve":
                    # Case of pub only
                    if item['name'] == "rust_main":
                        new_function_line = function_line.replace('pub fn', 'pub extern "C" fn')
                        modified_lines[start_line] = new_function_line
                        
                        body_start = start_line + 1
                        modified_lines.insert(body_start, " " * (indent + 4) + "initialize_tracing();\n")
                        modified_lines.insert(body_start + 1, " " * (indent + 4) + "let _span = tracing::info_span!(\"rust_main\").entered();\n\n")

                        modified_lines.insert(start_line, no_mangle_macro)
                        modified_lines.insert(start_line, trace_macro)

                        # Insert initialize_tracing function before rust_main
                        modified_lines.insert(start_line, "\n" + initialize_tracing_fn)
                    
                    else:
                        new_function_line = function_line.replace('pub fn', 'pub fn')
                        modified_lines[start_line] = new_function_line
                        #modified_lines.insert(start_line, no_mangle_macro)
                        modified_lines.insert(start_line, trace_macro)
                
                ########### From here onward
                else:
                    # Case of pub only
                    if item['name'] == "rust_main":
                        new_function_line = function_line
                        modified_lines[start_line] = new_function_line
                        
                        ########## Insert here
                        body_start = start_line + 1
                        modified_lines.insert(body_start, " " * (indent + 4) + "initialize_tracing();\n")
                        modified_lines.insert(body_start + 1, " " * (indent + 4) + "let _span = tracing::info_span!(\"rust_main\").entered();\n\n")
                        
                        ##########

                        modified_lines.insert(start_line, no_mangle_macro)
                        modified_lines.insert(start_line, trace_macro)

                        # Insert initialize_tracing function before rust_main
                        modified_lines.insert(start_line, "\n" + initialize_tracing_fn)
                    

            else:
                # Case of a normal function
                new_function_line = function_line.replace('fn', 'fn')
                modified_lines[start_line] = new_function_line
                modified_lines.insert(start_line, trace_macro)
                    
            print(f"Modified function: {name}")

        if item['category'] == "data_type":
            name = item['name']
            start_line = item['start_line']
                    
            # Get indentation of the function line
            function_line = modified_lines[start_line]
            indent = len(function_line) - len(function_line.lstrip())

            # Generate trace and no_mangle macros on separate lines
            trace_macro = " " * indent + "" #"#[derive(Debug)]\n"

            modified_lines.insert(start_line, trace_macro)


    # Write the modified content back to the file
    with open(file_path, 'w') as f:
        f.writelines(modified_lines)


def rust_filter_flow_log(log_path, out_json_path):
    function_calls = []
    
    # Definition of regex patterns
    input_pattern = re.compile(r"7GqX9B2: (\w+) @ ([^:]+):(\d+) input: (\w+) = (.+)")
    input_null_pattern = re.compile(r"7GqX9B2: (\w+) @ ([^:]+):(\d+) input: null")
    output_pattern = re.compile(r"7GqX9B2: (\w+) @ ([^:]+):(\d+) output = (.+)")
    
    # Read log file
    with open(log_path, 'r') as f:
        for line in f:
            line = line.strip()
            
            # Match input pattern (with arguments)
            match = input_pattern.match(line)
            if match:
                function_calls.append({
                    "function_name": match.group(1),
                    "file_path": match.group(2),
                    "line_number": int(match.group(3)),
                    "call_type": "input",
                    "value": f"{match.group(4)} = {match.group(5)}"
                })
                continue
            
            # Match input pattern (no arguments)
            match = input_null_pattern.match(line)
            if match:
                function_calls.append({
                    "function_name": match.group(1),
                    "file_path": match.group(2),
                    "line_number": int(match.group(3)),
                    "call_type": "input",
                    "value": "null"
                })
                continue
            
            # Match output pattern
            match = output_pattern.match(line)
            if match:
                function_calls.append({
                    "function_name": match.group(1),
                    "file_path": match.group(2),
                    "line_number": int(match.group(3)),
                    "call_type": "output",
                    "value": match.group(4)
                })
    
    # Write to JSON file
    with open(out_json_path, 'w', encoding='utf-8') as f:
        json.dump(function_calls, f, indent=4, ensure_ascii=False)

    print(f"Wrote out to {out_json_path}")



def modify_cargo(rust_io_dir, inspect_dir, inspect_core_dir):
    print(f"rust_io_dir: {rust_io_dir}")
    print(f"inspect_dir: {inspect_dir}")
    lib_name = os.path.basename(inspect_dir)
    core_lib_name = os.path.basename(inspect_core_dir)
    toml_path = f"{rust_io_dir}/Cargo.toml"
    
    # Read Cargo.toml
    with open(toml_path, 'r') as f:
        content = f.read()
    
    # Add [lib] section
    if '[lib]' not in content:
        # If [lib] section does not exist, add it
        lib_section = '\n[lib]\nname = "trans_rust"\ncrate-type = ["cdylib"]\n'
        # Add [lib] section after package section
        if '[package]' in content:
            parts = content.split('[package]')
            package_parts = parts[1].split('[', 1)
            content = '[package]' + package_parts[0] + lib_section + ('[' + package_parts[1] if len(package_parts) > 1 else '')

    # Find [dependencies] section
    if '[dependencies]' not in content:
        # If [dependencies] section does not exist, add it
        content += '\n[dependencies]\n'
    
    dependency_line = ""
    #f'''tracing = "0.1"
#tracing-subscriber = {{ version = "0.3", features = ["env-filter", "json"] }}'''

    # Check if dependency already exists
    if f'{lib_name}' not in content:
        # Add after [dependencies] section
        parts = content.split('[dependencies]')
        content = parts[0] + '[dependencies]\n' + dependency_line + (parts[1] if len(parts) > 1 else '')
    
    # Write changes back
    if FFI_STRATEGY == "preserve":
        with open(toml_path, 'w') as f:
            f.write(content)


def insert_func_inspector_lib(rust_io_dir, inspect_dir, inspect_core_dir):
    print(f"rust_io_dir: {rust_io_dir}")
    print(f"inspect_dir: {inspect_dir}")
    lib_name = os.path.basename(inspect_dir)
    core_lib_name = os.path.basename(inspect_core_dir)
    toml_path = f"{rust_io_dir}/Cargo.toml"
    
    # Read Cargo.toml
    with open(toml_path, 'r') as f:
        content = f.read()
    
    # Add [lib] section
    if '[lib]' not in content:
        # If [lib] section does not exist, add it
        lib_section = '\n[lib]\nname = "trans_rust"\ncrate-type = ["cdylib"]\n'
        # Add [lib] section after package section
        if '[package]' in content:
            parts = content.split('[package]')
            package_parts = parts[1].split('[', 1)
            content = '[package]' + package_parts[0] + lib_section + ('[' + package_parts[1] if len(package_parts) > 1 else '')

    # Find [dependencies] section
    if '[dependencies]' not in content:
        # If [dependencies] section does not exist, add it
        content += '\n[dependencies]\n'
    
    # Add function_inspector dependency
    dependency_line = f'{lib_name} = {{ path = "../{lib_name}" }}\n{core_lib_name} = {{ path = "../{core_lib_name}" }}'
    
    # Check if dependency already exists
    if f'{lib_name}' not in content:
        # Add after [dependencies] section
        parts = content.split('[dependencies]')
        content = parts[0] + '[dependencies]\n' + dependency_line + (parts[1] if len(parts) > 1 else '')
    
    # Write changes back
    with open(toml_path, 'w') as f:
        f.write(content)


####################################################
########## repair
####################################################

def get_called_data(rust_flow_path):
    rust_flow = read_json(rust_flow_path)
    
    moment_c_flows = []
    moment_rust_flows = []
    if rust_flow is None:
        return []

    for item in rust_flow:
        if item['lang'] == "C":
            moment_c_flows.append(item)
        
        elif item['lang'] == "Rust":
            moment_rust_flows.append(item)
    
    return moment_c_flows


report_template = f"""
{{
    "answer" : [
        {{
            "test_number" : 1 or 2 ... or 20,
            "error_log" : (log of the failed testcase),
            "need_function_flow" : True or False,
            "need_arg_return" : True or False,
            "need_module_deps" : True or False,
        }},
        {{
            "test_number" : 1 or 2 ... or 20,
            "error_log" : (log of the failed testcase),
            "need_function_flow" : True or False,
            "need_arg_return" : True or False,
            "need_module_deps" : True or False,
        }},...
    ],
    "ongoing" : true or false,
}}
"""

def ask_tetscase_report(error):
    
    prompt = []
    prompt.extend(["The following execution log was output when running 20 testcases.",
                   "Please follow the response rules and respond with JSON data summarizing the logs for the failing testcases."])

    prompt.extend(["", f"## Response rules:",
                  "- Insert one failing testcase number from 1-20 into the \"test_number\" key.",
                  "- Place the log output section for that test number into the \"error_log\" key value.",
                  "- For the following key values, set True if that information is needed for error fixing, or False if it is not needed.",
                  "    - \"need_function_flow\": Function call order",
                  "    - \"need_arg_return\": Argument and return values of called functions",
                  "    - \"need_module_deps\": Dependencies between modules",
                  f"- To avoid hitting the output token limit, please keep the JSON data in a single response to {output_max} tokens or less.", # For long responses,
                    #f"When responding in multiple parts, if there is more JSON data remaining, set the 'ongoing_in_mode' key to a boolean True. If the JSON data is the final portion, set the 'ongoing_in_mode' key to a boolean False.",  
                    "- If the JSON data in a single response is about to exceed the token limit, please split it across multiple responses.",
                    "- If the JSON data is the final portion, set the 'ongoing' key to a boolean False. If there is more JSON data remaining, set the 'ongoing' key to a boolean True.",
                  ])
    prompt.extend(["\n## Response format", "In summary, please respond in the following JSON format:"])
    prompt.extend([report_template])

    prompt.extend(["", "## Execution log:", error])

    exp_data = {}
    exp_data['experiment_path'] = "ask_report.c"
    exp_data['file_path'] = "ask_report.c" 
    exp_data['repair_count'] = 0
    exp_data['average'] = 0

    ongoing_flag = None
    sum_modified_list = []
    while (1):
        if ongoing_flag is True:
            print("Keep going to receive report.")

            prompt = []
            prompt.extend(["Please continue responding with JSON data summarizing the logs for the failing testcases."])

            prompt.extend(["",
                          "\n## Response format", 
                           "In summary, please respond in the following JSON format:"])
            prompt.extend([report_template])

            prompt.extend(["", "## Execution log:", error])
            
        rsp_json = ask_llm(prompt, "continue", llm_interface)

        if 'answer' in rsp_json:
            modified_list = rsp_json['answer']
            if not isinstance(modified_list, list):
                modified_list = [modified_list]
            sum_modified_list.extend(modified_list)
            
        if 'ongoing' in rsp_json:
            ongoing_flag = rsp_json['ongoing']
        else:
            print("Should include ongoing flag") 

        if not ongoing_flag:
            break
    
    if isinstance(sum_modified_list, (list, dict)):
        json_content = sum_modified_list  # Already a Python object, so loads is not needed
    else:
        json_content = json.loads(sum_modified_list)  # Use loads if it's a string

    return sum_modified_list



functional_template = f"""
# In "read_data" mode
{{
    "mode" : "read_data",
    "target_files" : [path/to/file1, path/to/file2, ..., path/to/fileN], 
    "file_slices" : (if necessary, otherwise None) [
        {{
            "file_path" : (file path),
            "start_line" : (start_line of the scope),
            "end_line" : (end_line of the scope),
        }},...
    ]
    "ongoing_in_mode" : true if the "answer" response in "read_data" mode is long and will continue in subsequent responses. false otherwise,
    "ongoing" : true if the response will continue in a different mode. false otherwise,
    "ready_to_execute" : True if modifications are complete and ready to execute for verification. False otherwise,
    "reason" : explanatory text for the response (insert here if needed)
}}


# In "modify_data" mode
{{
    "mode" : "modify_data",
    "answer" : [
        {{
            "file_path" : (file path),
            "start_line" : (start line of the original code to be deleted; must reflect the original range to be replaced),
            "end_line" : (end line of the original code to be deleted; must reflect the original range to be replaced),
        }},
        {{
            "file_path" : (file path),
            "start_line" : (start line of the original code to be deleted; must reflect the original range to be replaced),
            "end_line" : (end line of the original code to be deleted; must reflect the original range to be replaced),
        }},...
    ],
    "ongoing_in_mode" : true if the "answer" response in "modify_data" mode is long and will continue in subsequent responses. false otherwise,
    "ongoing" : true if the response will continue in a different mode. false otherwise,
    "ready_to_execute" : True if modifications are complete and ready to execute for verification. False otherwise,
    "reason" : explanatory text for the response (insert here if needed)
}}


# In "execute_command" mode
{{
    "mode" : "execute_command",
    "answer" : shell script content to be executed,
    "ongoing_in_mode" : true if the "answer" response in "execute_command" mode is long and will continue in subsequent responses. false otherwise,
    "ongoing" : true if the response will continue in a different mode. false otherwise,
    "ready_to_execute" : True if modifications are complete and ready to execute for verification. False otherwise,
    "reason" : explanatory text for the response (insert here if needed)
}}
"""


functional_modify_template = f"""
{{
    "mode" : "modify_data",
    "answer" : [
        {{
            "file_path" : (file path),
            "start_line" : (start line of the original code to be deleted; must reflect the original range to be replaced),
            "end_line" : (end line of the original code to be deleted; must reflect the original range to be replaced),
            "is_deletion" : True for deletion only, False for modification,
            "no_simplification" : true if all original intended features are fully preserved, without any omissions or simplifications. false otherwise,
            "is_JSON" :If the file_path is a JSON file, then True, otherwise False,
            "modified_data" : (Content of the corrected code without any omission. Content of the corrected code as a string if is_JSON is false, or as a direct JSON object if is_JSON is true.),
            "modification_part": (the number of the current part),
        }},
        {{
            "file_path" : (file path),
            "start_line" : (start line of the original code to be deleted; must reflect the original range to be replaced),
            "end_line" : (end line of the original code to be deleted; must reflect the original range to be replaced),
            "is_deletion" : True for deletion only, False for modification,
            "no_simplification" : true if all original intended features are fully preserved, without any omissions or simplifications. false otherwise,
            "is_JSON" :If the file_path is a JSON file, then True, otherwise False,
            "modified_data" : (Content of the corrected code without any omission. Content of the corrected code as a string if is_JSON is false, or as a direct JSON object if is_JSON is true.),
            "modification_part": (the number of the current part),
        }},...
    ],
    "ongoing_in_mode" : true if the "answer" response in "modify_data" mode is long and will continue in subsequent responses. false otherwise,
    "ongoing" : true if the response will continue in a different mode. false otherwise,
    "ready_to_execute" : True if modifications are complete and ready to execute for verification. False otherwise,
    "reason" : explanatory text for the response (insert here if needed)
}}
"""

summary_dict = {}
called_count = 0

# Proposed fix
def trim_code(target_path, file_code, given_limit, model="gpt-4"):
    if not os.path.isfile(target_path):
        return
    
    # Handle the case where file_code is None or is not a string
    if file_code is None:
        return
    
    # Convert to string type
    if not isinstance(file_code, str):
        file_code = str(file_code)
    
    encoder = tiktoken.encoding_for_model(model)
    full_tokens = len(encoder.encode(file_code))
    
    # The following is the same as the original code
    if full_tokens <= given_limit: # If it is already within the limit, return the whole thing
        print("Within limit")
        return file_code
    
    # Use binary search to find the largest portion that fits within the limit
    left, right = 0, len(file_code)
    best_length = 0
    
    while left <= right:
        mid = (left + right) // 2
        # Get the candidate substring
        candidate = file_code[:mid]
        # Calculate the number of tokens in that portion
        tokens = len(encoder.encode(candidate))
        
        if tokens <= given_limit - 50:  # Leave some margin for the omission message
            best_length = mid
            left = mid + 1
        else:
            right = mid - 1
    
    # Cut at the last complete line
    trimmed = file_code[:best_length]
    last_newline = trimmed.rfind('\n')
    if last_newline != -1:
        trimmed = trimmed[:last_newline + 1]
    
    # Add omission message (token-count based)
    remaining_tokens = full_tokens - len(encoder.encode(trimmed))
    trimmed += f"\n... ( remaining_tokens is {remaining_tokens}.) Exceeding token limit, content truncated. To view the complete content of {target_path}, please use read_data mode and set file_slice (specified range) to read each section separately."
    
    return trimmed



def repair_semantics(repair_target, interface):

    llm_interface = interface.llm_interface
    output_max = llm_interface.output_max

    mix_io_dir = interface.mix_io_dir
    exp_data = interface.exp_data
    c_io_dir = interface.c_io_dir
    rust_io_dir = interface.rust_io_dir
    repair_count = interface.repair_count
    work_dir = interface.work_dir
    database_dir = interface.database_dir
    flow_on = interface.flow_on

    target = interface.target

    meta_dir = interface.meta_dir
    dep_json_path = interface.dep_json_path
    
    rust_build_path = interface.rust_build_path
    build_path = interface.build_path
    run_test_path = interface.run_test_path
    run_all_path = interface.run_all_path

    rust_c_path = interface.rust_c_path
    c_rust_path = interface.c_rust_path

    test_number = interface.test_number
    error_log = interface.error_log
    flow_path = interface.flow_path

    error = interface.error
    std_out = interface.std_out

    run_path = run_all_path

    rust_c_map = read_json(rust_c_path)
    c_rust_path = read_json(c_rust_path)

    execute_path = f"{mix_io_dir}/execute.sh"
    if not os.path.exists(execute_path):
        create_permissioned_file(execute_path)

    execute_dir = os.path.dirname(os.path.normpath(execute_path))

    modified_c_keys = set()
    modified_rust_lines = []
    editied_files = []
    judge_count = 0

    mode = None
    execute_error = None
    execute_out = None
    read_prompt = None

    ongoing_flag = None #False
    mode = None

    ready_to_execute = None

    modified_files = set()

    while (1):

        if exp_data['repair_count'] == REPAIR_MAX:
            print(f"Force to finish. Hit the REPAIR_MAX ({REPAIR_MAX}).")
            iteration_dict[repair_target] = repair_count
            sys.exit(1)  #return True

        """
        if mode != "read_data":
            if (repair_count != 1): #if (repair_count != 1 and (repair_target == "build" or repair_target == "compile")): # これは、repair_count != 1じゃないよね？
                error, std_out, repair_count = run_script(run_path, 100, True, None, "both", None, repair_count, None, None, mode)
                judge_count += 1
                print(f"Judging at run_script: error: {error}")

        print(f"Judging at {repair_count}: run_path: {run_path} mode: {mode}, ongoing_flag: {ongoing_flag}, error: {error}")
        if error is None and mode != "read_data":  # This feels like a big change though  # if error is None and mode != "read_data" and ongoing_flag is False:
            break
        """

        print(f"Judging at {repair_count}: mode: {mode}, ongoing_flag: {ongoing_flag}") #, error: {error}")
        if mode != "read_data" and ready_to_execute is True: #ongoing_flag is False: # error is None and 
            break

        if repair_target == "semantics":
            if repair_count == 1:

                prompt = []
                if error_log is not None:

                    if FFI_STRATEGY == "preserve":
                        prompt.extend([f"We have translated a memory-vulnerable C program ({c_io_dir}) to a memory-safe Rust program ({rust_io_dir}).",
                                    f"To ensure equivalence between the pre- and post-translation code, we called the Rust main function via FFI from C test cases, but test{test_number} is not passing.",
                                    f"Please modify the Rust program ({rust_io_dir}) to fundamentally resolve the error and make test{test_number} pass.",
                                    f"For incremental fixes, please provide a solution specifically for test{test_number} at this time.",
                                    "When answering, please follow the response rules below and generate a response using only one of the following three response modes:",
                                ])  
                    else:
                         prompt.extend([f"We have translated a memory-vulnerable C program ({c_io_dir}) to a memory-safe Rust program ({rust_io_dir}).",
                                    f"To ensure equivalence between the pre- and post-translation code, we conducted the same testcases for both C and Rust programs, but test{test_number} is not passing.",
                                    f"Please modify the Rust program ({rust_io_dir}) to fundamentally resolve the error and make test{test_number} pass.",
                                    f"For incremental fixes, please provide a solution specifically for test{test_number} at this time.",
                                    "When answering, please follow the response rules below and generate a response using only one of the following three response modes:",
                                ])                   
                else:
                    if FFI_STRATEGY == "preserve":
                        prompt.extend([f"We have translated a memory-vulnerable C program ({c_io_dir}) to a memory-safe Rust program ({rust_io_dir}).",
                                        "To ensure equivalence between the pre- and post-translation code, we called the Rust main function via FFI from C test cases, but errors are occurring.",
                                        f"Please modify the Rust program ({rust_io_dir}) to fundamentally resolve the errors.",
                                        "When answering, please follow the response rules below and generate a response using only one of the following three response modes:",                               
                                        ])
                    else:
                        prompt.extend([f"We have translated a memory-vulnerable C program ({c_io_dir}) to a memory-safe Rust program ({rust_io_dir}).",
                                        f"To ensure equivalence between the pre- and post-translation code, we conducted the same testcases for both C and Rust programs, but test{test_number} is not passing.",
                                        f"Please modify the Rust program ({rust_io_dir}) to fundamentally resolve the errors.",
                                        "When answering, please follow the response rules below and generate a response using only one of the following three response modes:",                               
                                        ])
                if FFI_STRATEGY == "preserve":
                    prompt.extend(["",
                                "## Response rules:",
                                "- Before making any modifications, please check the original C program to ensure all functionality is properly implemented. If any features have been simplified or are missing, please modify the Rust program to faithfully implement all the original functionality from the C code.",
                                "- Please do NOT create function Rust implementations based on your own assumptions, without knowing its C implementaion. Always find and reference the corresponding function in the original C code before writing its Rust equivalent.",
                                "- To identify the problematic areas, first thoroughly understand the target program.",
                                "- Afterwards, please identify and fix the root cause of the error according to the following rules:",
                                #"- Please identify and fix the root cause of the error according to the following principles:",
                                #"- Please use the "
                                f"    - If the root cause can be identified from error logs (compile or runtime error messages), please fix the identified location.",
                                f"    - If there are no compile or runtime error messages, and the test case is failing due to differences between expected and actual results:", #f"    - Otherwise, to clear the root cause, please use the following info as needed and identify the cause code.", #f"    - If there are no compile or runtime error messages, and the test case is failing due to differences between expected and actual results:",
                                #f"        -  Please debug by yourself.",
                                f"        - Please refer to the flow_results directory, which contains information about function execution flows for the current Rust implementations for each testcase.", #, arguments, and return values for both the original C and current Rust implementations for each testcase.",
                                #f"        - Please refer to the {mix_io_dir}/flows directory, which contains information about function execution flows, arguments, and return values for both the original C and current Rust implementations for each testcase.",
                                #f"        - This flow information is updated whenever the Rust program is modified. So, please check the current status as needed.",
                                f"        - For your reference, the test{test_number}_golden_flow.txt file contains only the expected execution flows of the C implementation for each testcase.", 
                                #f"- The entry point in the Rust program is in workspace_io/trans_rust/src/which_2_21/module4_h/unit0.rs.",
                                f"- When fixing the Rust program, please aim to understand the original C program's intent and functionality accurately, and implement an equivalent code, rather than creating special handling or hacky fixes for specific test cases.",
                                f"- The purpose of converting from C to Rust is to replace memory-vulnerable code with memory-safe code. When modifying the Rust program ({rust_io_dir}), please suggest safe modifications that avoid using unsafe blocks and raw pointers whenever possible.",
                                f"- The {rust_build_path} shell file contains code for building the Rust program.",
                                f"- The {build_path} shell file contains code for building the C program.",
                                f"- The {run_test_path} shell file contains code for executing the C program's test cases.",
                                f"- The {run_all_path} shell file contains code that executes {rust_build_path}, {build_path} and {run_test_path} together.",
                                f"- When making modifications, please consider the directory structure where the Rust program ({rust_io_dir}) is located.",
                                f"- As the goal is to maintain end-to-end test equivalence between the original C program and its Rust conversion, absolutely do not modify the existing C test cases ({run_test_path}) or C program ({c_io_dir}).",
                            ])
                else:
                    prompt.extend(["",
                                "## Response rules:",
                                "- Before making any modifications, please check the original C program to ensure all functionality is properly implemented. If any features have been simplified or are missing, please modify the Rust program to faithfully implement all the original functionality from the C code.",
                                "- Please do NOT create function Rust implementations based on your own assumptions, without knowing its C implementaion. Always find and reference the corresponding function in the original C code before writing its Rust equivalent.",
                                "- To identify the problematic areas, first thoroughly understand the target program.",
                                "- Afterwards, please identify and fix the root cause of the error according to the following rules:",
                                #"- Please identify and fix the root cause of the error according to the following principles:",
                                #"- Please use the "
                                f"    - If the root cause can be identified from error logs (compile or runtime error messages), please fix the identified location.",
                                f"    - If there are no compile or runtime error messages, and the test case is failing due to differences between expected and actual results:", #f"    - Otherwise, to clear the root cause, please use the following info as needed and identify the cause code.", #f"    - If there are no compile or runtime error messages, and the test case is failing due to differences between expected and actual results:",
                                #f"        -  Please debug by yourself.",
                                f"        - Please refer to the flow_results directory, which contains information about function execution flows for the current Rust implementations for each testcase.", #, arguments, and return values for both the original C and current Rust implementations for each testcase.",
                                #f"        - This flow information is updated whenever the Rust program is modified. So, please check the current status as needed.",
                                f"        - For your reference, the test{test_number}_golden_flow.txt file contains only the expected execution flows of the C implementation for each testcase.", 
                                #f"- The entry point in the Rust program is in workspace_io/trans_rust/src/which_2_21/module4_h/unit0.rs.",
                                f"- When fixing the Rust program, please aim to understand the original C program's intent and functionality accurately, and implement an equivalent code, rather than creating special handling or hacky fixes for specific test cases.",
                                f"- The purpose of converting from C to Rust is to replace memory-vulnerable code with memory-safe code. When modifying the Rust program ({rust_io_dir}), please suggest safe modifications that avoid using unsafe blocks and raw pointers whenever possible.",
                                f"- The {rust_build_path} shell file contains code for building the Rust program.",
                                #f"- The {o_run_path} shell file contains code for building the C program.",
                                f"- The {run_test_path} shell file contains code for executing the test cases.",
                                f"- The {run_all_path} shell file contains code that executes {rust_build_path} and {run_test_path} together.",
                                f"- When making modifications, please consider the directory structure where the Rust program ({rust_io_dir}) is located.",
                                f"- As the goal is to maintain end-to-end test equivalence between the original C program and its Rust conversion, absolutely do not modify the existing C test cases ({run_test_path}) or C program ({c_io_dir}).",
                            ])

                prompt.extend(["- When encountering errors with backslashes in byte literals, you need to escape the backslash in both the source code and within the byte literal. Therefore, use three backslashes (double backslash).",
                            "- When encountering errors with backslashes in character literals, you need to escape the backslash in both the source code and within the character literal. Therefore, use two backslashes.",
                            f"- To avoid output token limitations, please keep JSON data within {output_max} tokens in a single response",
                            "- If a single mode response might exceed the token limit, please split the response into multiple parts.",
                            "- If this JSON data is the final part, set the 'ongoing_in_mode' key to False. If there is more JSON data remaining, set the 'ongoing_in_mode' key to True.",
                            "- Always generate responses using only one mode ('read_data', 'modify_data', or 'execute_command'), and use 'ongoing_in_mode' only when further interaction is needed within that single mode.",
                            "- If you want to switch modes, end the current mode by setting ongoing_in_mode to false",
                            #"- When responding in 'modify_data' mode, the modified_code content must not contain any omissions as it will be copied directly.",
                        ])

            else:
                # important
                if ongoing_flag is True:
                    if error_log is not None:
                        prompt = [f"Please continue with the modifications to the Rust program ({rust_io_dir}) to make test case {test_number} pass.",
                                  "Please continue to follow the provided response rules."                                ]
                            
                    else:
                        prompt = [f"Please continue with the modifications to the Rust program ({rust_io_dir}) to make it executable with the test cases.",
                                  "Please continue to follow the provided response rules."
                                ]

                else:
                    print(f"Ongoing_flag here: {ongoing_flag}")
                    if error_log is not None:
                        prompt = [f"Test case execution of the Rust program ({rust_io_dir}) shows that case {test_number} is not passing.",
                                f"Please continue to fundamentally resolve the errors and modify the Rust program ({rust_io_dir}) to make test case {test_number} pass.",
                                f"For incremental fixes, please provide a solution specifically for test{test_number} at this time.",
                                "When answering, please follow the response rules below and generate a response using only one of the following three response modes.",
                            ]
                    else:
                        prompt = [f"Test case execution of the Rust program ({rust_io_dir}) is resulting in errors.",
                                f"Please continue to fundamentally resolve the errors and modify the Rust program ({rust_io_dir}) to make it executable with the test cases.",
                                "If multiple test cases are failing, focus on fixing the first failing test case first before moving on to the others.",
                                "When answering, please follow the response rules below and generate a response using only one of the following three response modes.",
                            ]

                    if FFI_STRATEGY == "preserve":
                        prompt.extend(["",
                                "## Response rules:",
                                "- If any features have been simplified or are missing in the translated Rust program, please modify the Rust program to faithfully implement all the original functionality from the C code.",
                                "- Please identify and fix the root cause of the error according to the following rules:",
                                f"    - If the root cause can be identified from error logs (compile or runtime error messages), please fix the identified location.",
                                f"    - If there are no compile or runtime error messages, and the test case is failing due to differences between expected and actual results:",
                                f"        -  Please debug by yourself.",
                                f"    - Otherwise, to clear the root cause, please use the following info as needed and identify the cause code.", #f"    - If there are no compile or runtime error messages, and the test case is failing due to differences between expected and actual results:",
                                f"        - Please refer to the {flow_path} file, which contains information about function execution flow for both the original C and current Rust implementations.",
                                #f"        - Please refer to the {flow_path} file, which contains information about function execution flow, arguments, and return values for both the original C and current Rust implementations.",
                                f"        - This flow information is updated whenever the Rust program is modified. So, please check the current status as needed.",
                                f"        - For your reference, the golden/test{test_number}_golden_flow.txt file contains the expected execution flow of the C implementation.",  #  the {c_io_dir}/{target}/golden/test{test_number}_golden_flow.txt file contains only the expected execution flow of the C implementation.", 
                                #f"- The entry point in the Rust program is in workspace_io/trans_rust/src/which_2_21/module4_h/unit0.rs",
                                f"- When fixing the Rust program, please aim to understand the original C program's intent and functionality accurately, and implement an equivalent code, rather than creating special handling or hacky fixes for specific test cases.",
                                f"- The purpose of converting from C to Rust is to replace memory-vulnerable code with memory-safe code. When modifying the Rust program ({rust_io_dir}), please suggest safe modifications that avoid using unsafe blocks and raw pointers whenever possible.",
                                f"- The {rust_build_path} shell file contains code for building the Rust program.",
                                f"- The {build_path} shell file contains code for building the C program.",
                                f"- The {run_test_path} shell file contains code for executing the C program's test cases.",
                                f"- The {run_all_path} shell file contains code that executes {rust_build_path}, {build_path} and {run_test_path} together.",
                                f"- When making modifications, please consider the directory structure where the Rust program ({rust_io_dir}) is located.",
                                f"- As the goal is to maintain end-to-end test equivalence between the original C program and its Rust conversion, absolutely do not modify the existing C test cases ({run_test_path}) or C program ({c_io_dir}).",
                            ])
                    else:
                        if flow_on is True:
                            prompt.extend(["",
                                    "## Response rules:",
                                    "- If any features have been simplified or are missing in the translated Rust program, please modify the Rust program to faithfully implement all the original functionality from the C code.",
                                    "- Please identify and fix the root cause of the error according to the following rules:",
                                    f"    - If the root cause can be identified from error logs (compile or runtime error messages), please fix the identified location.",
                                    f"    - If there are no compile or runtime error messages, and the test case is failing due to differences between expected and actual results:",
                                    f"        -  Please debug by yourself.",
                                    f"    - Otherwise, to clear the root cause, please use the following info as needed and identify the cause code.", #f"    - If there are no compile or runtime error messages, and the test case is failing due to differences between expected and actual results:",
                                    f"        - Please refer to the {flow_path} file, which contains information about function execution flow for both the original C and current Rust implementations.",
                                    #f"        - Please refer to the {flow_path} file, which contains information about function execution flow, arguments, and return values for both the original C and current Rust implementations.",
                                    f"        - This flow information is updated whenever the Rust program is modified. So, please check the current status as needed.",
                                    f"        - For your reference, the golden/test{test_number}_golden_flow.txt file contains the expected execution flow of the C implementation.",  #  the {c_io_dir}/{target}/golden/test{test_number}_golden_flow.txt file contains only the expected execution flow of the C implementation.", 
                                    #f"- The entry point in the Rust program is in workspace_io/trans_rust/src/which_2_21/module4_h/unit0.rs",
                                    f"- When fixing the Rust program, please aim to understand the original C program's intent and functionality accurately, and implement an equivalent code, rather than creating special handling or hacky fixes for specific test cases.",
                                    f"- The purpose of converting from C to Rust is to replace memory-vulnerable code with memory-safe code. When modifying the Rust program ({rust_io_dir}), please suggest safe modifications that avoid using unsafe blocks and raw pointers whenever possible.",
                                    f"- The {rust_build_path} shell file contains code for building the Rust program.",
                                    #f"- The {build_path} shell file contains code for building the C program.",
                                    f"- The {run_test_path} shell file contains code for executing the test cases.",
                                    f"- The {run_all_path} shell file contains code that executes {rust_build_path}, {build_path} and {run_test_path} together.",
                                    f"- When making modifications, please consider the directory structure where the Rust program ({rust_io_dir}) is located.",
                                    f"- As the goal is to maintain end-to-end test equivalence between the original C program and its Rust conversion, absolutely do not modify the existing C test cases ({run_test_path}) or C program ({c_io_dir}).",
                                ])

                        else:
                            prompt.extend(["",
                                    "## Response rules:",
                                    "- If any features have been simplified or are missing in the translated Rust program, please modify the Rust program to faithfully implement all the original functionality from the C code.",
                                    "- Please identify and fix the root cause of the error according to the following rules:",
                                    f"    - If the root cause can be identified from error logs (compile or runtime error messages), please fix the identified location.",
                                    f"    - If there are no compile or runtime error messages, and the test case is failing due to differences between expected and actual results:",
                                    f"        -  Please debug by yourself.",
                                    # f"    - Otherwise, to clear the root cause, please use the following info as needed and identify the cause code.",
                                    # f"        - Please refer to the {flow_path} file, which contains information about function execution flow for both the original C and current Rust implementations.",
                                    # f"        - For your reference, the golden/test{test_number}_golden_flow.txt file contains the expected execution flow of the C implementation.", 
                                    f"- When fixing the Rust program, please aim to understand the original C program's intent and functionality accurately, and implement an equivalent code, rather than creating special handling or hacky fixes for specific test cases.",
                                    f"- The purpose of converting from C to Rust is to replace memory-vulnerable code with memory-safe code. When modifying the Rust program ({rust_io_dir}), please suggest safe modifications that avoid using unsafe blocks and raw pointers whenever possible.",
                                    f"- The {rust_build_path} shell file contains code for building the Rust program.",
                                    #f"- The {build_path} shell file contains code for building the C program.",
                                    f"- The {run_test_path} shell file contains code for executing the test cases.",
                                    f"- The {run_all_path} shell file contains code that executes {rust_build_path}, {build_path} and {run_test_path} together.",
                                    f"- When making modifications, please consider the directory structure where the Rust program ({rust_io_dir}) is located.",
                                    f"- As the goal is to maintain end-to-end test equivalence between the original C program and its Rust conversion, absolutely do not modify the existing C test cases ({run_test_path}) or C program ({c_io_dir}).",
                                ])

                    prompt.extend(["- When encountering errors with backslashes in byte literals, you need to escape the backslash in both the source code and within the byte literal. Therefore, use three backslashes (double backslash).",
                            "- When encountering errors with backslashes in character literals, you need to escape the backslash in both the source code and within the character literal. Therefore, use two backslashes.",
                            f"- To avoid output token limitations, please keep JSON data within {output_max} tokens in a single response",
                            "- If a single mode response might exceed the token limit, please split the response into multiple parts.",
                            "- If this JSON data is the final part, set the 'ongoing_in_mode' key to False. If there is more JSON data remaining, set the 'ongoing_in_mode' key to True.",
                            "- Always generate responses using only one mode ('read_data', 'modify_data', or 'execute_command'), and use 'ongoing_in_mode' only when further interaction is needed within that single mode.",
                            "- If you want to switch modes, end the current mode by setting ongoing_in_mode to false",
                            #"- When responding in 'modify_data' mode, the modified_code content must not contain any omissions as it will be copied directly.",
                        ])
                            

        if execute_error is not None or execute_out is not None:
            if execute_out is not None:
                write_file(f"{database_dir}/execute_out.txt", execute_out)
                execute_out = trim_code(f"{database_dir}/execute_out.txt", execute_out, 8000) #10000)
                prompt.extend(["\n## Execution result:",
                            "The result executed in execute_command mode is as follows:"
                            f"{execute_out}",
                            ""])
                execute_out = None

            if execute_error is not None:
                prompt.extend(["\n## Execution result error: ",
                            "The result error executed in execute_command mode is as follows:"
                            f"{execute_error}"])
                execute_error = None # Initialization
        
        if read_prompt is not None:
            prompt.extend(read_prompt)
            read_prompt = None # Initialization

        print(f"ongoing_flag is {ongoing_flag}")

        if ongoing_flag is False or ongoing_flag is None:
            prompt.extend(["",
                        "## Response modes",
                        "Please select only one of the three modes when responding and generate your response accordingly.",
                        "",
                        "1. In 'read_data' mode:",
                        "### Purpose:",
                        "- Returns the content of the specified file as it is.",
                        "### Format:",
                        "- Include the file path you want to read inside the \"target_files\" key in the JSON data.",
                        "- If the number of lines in the file you want to read is too large and cannot be viewed due to context window limitations, you can specify \"start_line\" and \"end_line\" along with file_path in \"file_slices\" to know about that specific range of lines.",
                        "",
                        "2. In 'modify_data' mode:",
                        "### Purpose:",
                        "- Allows modifying an existing file at the specified file path.",
                        "### Format:",
                        "- Insert the filename, start line, and end line of the change location to be inserted into the values of the \"file_path\", \"start_line\", and \"end_line\" keys in JSON format data.",
                        f"- For file_path, write a relative path in the format '{work_dir}/path/to/file'", #f"- For file_path, write a relative path starting from {work_dir}.",
                        "- Since we will ask about the actual modifications later, for now, please only specify the \"start_line\" and \"end_line\" that need modification. Do not write \"modified_data\".",
                        "",
                        "3. In 'execute_command' mode:",
                        "### Purpose:",
                        "- Executes the provided shell script code.",
                        "### Format:",
                        #f"- This executes separately from {run_path}. If not necessary beyond {run_path}, you do not need to include it in the response.",
                        "- Put the shell script code to be executed in the \"answer\" field of the JSON format data.",
                        f"- The answered shell script code will be saved in the shell script file at {execute_path} and runs from the {execute_dir} directory automatically, therefore:",
                        f"    - You don't need to include the command to move to the {execute_dir} directory like \"cd {execute_dir}\".",
                        f".   - All the path written in {execute_path} should be relative to that."
                        "- The execution of ./execute.sh should not have any arguments.",
                        "- The shell script can include multiple commands."
                    ])       
            
            prompt.extend(["\n## Response format", "In summary, please respond in the following JSON format:"]) 
            prompt.extend([functional_template])

            test_code = get_lined_code(run_test_path, mix_io_dir)
            prompt.extend(["", f"## Executed test case ({run_test_path}):", f"{test_code}"])

            prompt.extend(["", f"## Error in test{test_number}:", f"{error_log}"]) # error])

            if error is not None:
                prompt.extend(["", "## Execution error:",
                                f"{error}"])
                
                write_file(f"{database_dir}/std_out.txt", std_out)
                std_out = trim_code(f"{database_dir}/std_out.txt", std_out, 8000) #10000)
                prompt.extend(["", "## Standard output of execution:", 
                                    f"{std_out}"])
                std_out = None # Initialize
                error = None

            """
            if log_choice['need_function_flow'] is True and log_choice['need_arg_return'] is False:
                flow_path = f"{mix_io_dir}/flows/test{test_number}.txt"
                if os.path.exists(flow_path):
                    prompt.extend(["", f"## Function execution flow:"])
                    flow_code = read_file(flow_path)
                    prompt.extend([flow_code])
            
            elif log_choice['need_arg_return'] is True:
                value_path = f"{mix_io_dir}/analysis/test{test_number}.json"
                if os.path.exists(value_path):
                    prompt.extend(["", f"## Function arguments and return values:"])
                    value_code = read_file(value_path)
                    prompt.extend([value_code])
            
            if log_choice['need_module_deps'] is True:
                prompt.extend(["", f"## Module structure of the target Rust program ({rust_io_dir}):"])
                structure = get_cargo_modules(rust_io_dir)
                prompt.extend([structure])
            """

            prompt.extend(["", f"## Directory structure of C program ({c_io_dir}) using functions from Rust program ({rust_io_dir}):"])
            directory_structure = get_dir_struct('s_repair', mix_io_dir, None)  # c_io_dir)
            prompt.extend([directory_structure])
            

        ################################################

        prompt = adjust_prompt(prompt)
        print("-------------------------")

        print(f"repair_target: {repair_target}")
        exp_data['repair_count'] =  repair_count

        if repair_count == 1:
            rsp_json = ask_llm(prompt, "init", llm_interface)
        else:
            delete_file(execute_path)
            create_permissioned_file(execute_path)
            rsp_json = ask_llm(prompt, "continue", llm_interface)


        ################################################
        #ongoing_flag = False
        ongoing_in_mode_flag = False

        sum_target_list = []
        sum_modified_list = []
        sum_deleted_list = []

        sum_slice_list = []

        while (1):
            prompt = []
            execute_error = None

            print(f"ongoing_flag is {ongoing_flag}")
            if 'ongoing' in rsp_json:
                ongoing_flag = rsp_json['ongoing']

            if 'ongoing_in_mode' in rsp_json:
                ongoing_in_mode_flag = rsp_json['ongoing_in_mode']

            if 'ready_to_execute' in rsp_json:
                ready_to_execute = rsp_json['ready_to_execute']

            if 'mode' in rsp_json:
                mode = rsp_json['mode']

                if mode == 'modify_data':
                    if 'answer' in rsp_json:
                        modified_list = rsp_json['answer'] # Maybe it's okay to put the individually converted ones here
                        if not isinstance(modified_list, list):
                            modified_list = [modified_list]
                        sum_modified_list.extend(modified_list)

                        #if ongoing_in_mode_flag is False:
                        sequences = []
                        seen_sequences = set()
                        for mod in sum_modified_list:
                            if mod['file_path'] not in seen_sequences:
                                sequences.append(mod['file_path'])
                                seen_sequences.add(mod['file_path'])

                        # continuation
                        seq_string = ""
                        i = 0
                        for seq_path in sequences:
                            if i != 0:
                                seq_string += ", "
                            seq_string += seq_path
                            i += 1

                        prompt = []
                        prompt.extend([f"Please write the actual modifications for the file {seq_string} in modify_data mode.", #prompt.extend([f"引き続き、modified_dataのモードで、{seq_string}のファイルの実際の修正の内容を書いてください。",
                                        "",
                                        "## Response rules:", 
                                        "- Please insert the filename, start line, and end line of the section to be deleted into the \"file_path\", \"start_line\", and \"end_line\" keys in the JSON data.",
                                        "- Then, insert the new content that should be inserted at that [start_line] into the value of the \"modified_data\" key.",
                                        "- Detailed modification process is as follows. Please carefully write start_line, end_line and modified_data considering the process:",
                                        "    1. All code in the specified range (from [start_line] to [end_line]) will be completely deleted.",
                                        "    2. The content you provide in \"modified_data\" will be inserted at [start_line].",
                                        "    3. All code from [end_line + 1] onwards will remain unchanged and be appended after your modified_data.",
                                        #"- Please use the exact line numbers shown on the left side of the code (Line X) for start_line and end_line.",
                                        "- Please use the exact line numbers and indentation levels shown on the left side of the code (Line X [Y], where X is the line number and Y is the indentation level) for start_line, end_line and modified_data.",
                                        "- In case the modification content (modified_data) for a single range (start_line-end_line) is too long to include in one entry:",
                                        "    - Please split it across multiple answer entries.",
                                        "    - Each of these answer entries should maintain the same file_path, start_line, and end_line values",
                                        "    - Include modification_part representing the number of the current part in each entry to track the split.",
                                        #"    - Include modification_part information in each entry to track the split:",
                                        #"        - current: the number of the current part:",
                                        #"        - total: the total number of parts",
                                        "    - please remain ongoing_in_mode and ongoing flags true until all parts are delivered",       
                                        #"- Please insert the filename, start line, and end line of the section to be deleted and modified into the \"file_path\", \"start_line\", and \"end_line\" keys in the JSON data.",
                                        #"- Then, insert the new content that should be inserted at that modification location into the value of the \"modified_data\" key.",
                                        f"- For file_path, write the relative path starting from {work_dir}.",
                                        "- Insert appropriate indentation in modified_data so that it can be executed correctly when copied and pasted into the original code's location from start_line to end_line.",
                                        "- \"modified_data\" must not contain any omissions, as it will be directly inserted and executed in the original code.",
                                        "- If you want to only perform deletion without inserting into a specific location in the existing specified file path, set the value of 'is_deletion' to True.",
                                        "- If you want to overwrite the entire file rather than just modifying the specified line range, set the value of 'overwrite_all' to True.",
                                        "- Set the value of \"no_simplification\" to True if the functionality intended before modification exists completely without omission or simplification. Set it to False otherwise.",
                                        "- If the target file for editing is a JSON file, set the \"is_JSON\" flag to True and insert the modified JSON data into \"modified_data\"",
                                        "- In modifications, please avoid using unsafe, and use the Rust standard library or crates to achieve equivalent functionality in a safe manner.",
                                        "- When representing backslashes as byte literals, escape the backslash twice in the source code, and also escape it again in the byte literal, resulting in four backslashes (double backslashes).",
                                        "- When representing backslashes as character literals, escape the backslash once in the source code and again in the character literal, resulting in two backslashes.",
                                        "- Please ensure that the prefix to all function names in the Rust program is \"rust_\".",
                                        ])

                        prompt.extend(["\n## Response format", "In summary, please respond in the following JSON format:"])
                        prompt.extend([functional_modify_template])

                        for seq_path in sequences:
                            seq_code = get_lined_code(seq_path, mix_io_dir)
                            prompt.extend(["", f"## Code in {seq_path}:", seq_code])

                        if error_log is not None:
                            prompt.extend(["", f"## Error:", error_log])

                        sum_modified_list = []
                        mod_count = 1
                        while(1):

                            child_rsp_json = ask_llm(prompt, "continue", llm_interface)

                            # 'ongoing' needs to be updated here
                            if 'ongoing' in child_rsp_json:
                                ongoing_flag = child_rsp_json['ongoing']

                            if 'ongoing_in_mode' in child_rsp_json:
                                ongoing_in_mode_flag = child_rsp_json['ongoing_in_mode']
                            
                            if 'ready_to_execute' in child_rsp_json:
                                ready_to_execute = child_rsp_json['ready_to_execute']

                            print(f"ongoing_flag at location 2 is {ongoing_flag}")
                            print(f"ongoing_in_mode_flag at location 2 is {ongoing_in_mode_flag}")

                            if 'mode' in child_rsp_json:
                                mode = child_rsp_json['mode']

                                if mode == 'modify_data':
                                    if 'answer' in child_rsp_json:
                                        modified_list = child_rsp_json['answer']
                                        if not isinstance(modified_list, list):
                                            modified_list = [modified_list]
                                        sum_modified_list.extend(modified_list)

                                else:
                                    rsp_json = child_rsp_json

                            ready_to_execute = False
                            if 'ready_to_execute' in child_rsp_json:
                                ready_to_execute = child_rsp_json['ready_to_execute']

                            if ready_to_execute is True or ongoing_in_mode_flag is False:
                                print("Breaking in child modifying loop") 
                                break

                            prompt = []

                            prompt.extend([f"Continue writing the actual modifications for the file {seq_string} in modify_data mode.",
                                    "",
                                    "## Response rules:", 
                                    "- Please insert the filename, start line, and end line of the section to be deleted into the \"file_path\", \"start_line\", and \"end_line\" keys in the JSON data.",
                                    "- Then, insert the new content that should be inserted at that [start_line] into the value of the \"modified_data\" key.",
                                    "- Detailed modification process is as follows. Please carefully write start_line, end_line and modified_data considering the process:",
                                    "    1. All code in the specified range (from [start_line] to [end_line]) will be completely deleted.",
                                    "    2. The content you provide in \"modified_data\" will be inserted at [start_line].",
                                    "    3. All code from [end_line + 1] onwards will remain unchanged and be appended after your modified_data.",
                                    #"- Please use the exact line numbers shown on the left side of the code (Line X) for start_line and end_line.",
                                    "- Please use the exact line numbers and indentation levels shown on the left side of the code (Line X [Y], where X is the line number and Y is the indentation level) for start_line, end_line and modified_data.",
                                    "- In case the modification content (modified_data) for a single range (start_line-end_line) is too long to include in one entry:",
                                    "    - Please split it across multiple answer entries.",
                                    "    - Each of these answer entries should maintain the same file_path, start_line, and end_line values",
                                    "    - Include modification_part representing the number of the current part in each entry to track the split.",
                                    #"    - Include modification_part information in each entry to track the split:",
                                    #"        - current: the number of the current part:",
                                    #"        - total: the total number of parts",
                                    "    - please remain ongoing_in_mode and ongoing flags true until all parts are delivered",                      
                                    #"- Please insert the filename, start line, and end line of the section to be deleted and modified into the \"file_path\", \"start_line\", and \"end_line\" keys in the JSON data.",
                                    #"- Then, insert the new content that should be inserted at that modification location into the value of the \"modified_data\" key.",
                                    f"- For file_path, write the relative path starting from {work_dir}.",
                                    "- Insert appropriate indentation in modified_data so that it can be executed correctly when copied and pasted into the original code's location from start_line to end_line.",
                                    "- \"modified_data\" must not contain any omissions, as it will be directly inserted and executed in the original code.",
                                    "- If you want to only perform deletion without inserting into a specific location in the existing specified file path, set the value of 'is_deletion' to True.",
                                    "- If you want to overwrite the entire file rather than just modifying the specified line range, set the value of 'overwrite_all' to True.",
                                    "- Set the value of \"no_simplification\" to True if the functionality intended before modification exists completely without omission or simplification. Set it to False otherwise.",
                                    "- If the target file for editing is a JSON file, set the \"is_JSON\" flag to True and insert the modified JSON data into \"modified_data\"",
                                    "- In modifications, please avoid using unsafe, and use the Rust standard library or crates to achieve equivalent functionality in a safe manner.",
                                    "- When representing backslashes as byte literals, escape the backslash twice in the source code, and also escape it again in the byte literal, resulting in four backslashes (double backslashes).",
                                    "- When representing backslashes as character literals, escape the backslash once in the source code and again in the character literal, resulting in two backslashes.",
                                    "- Please ensure that the prefix to all function names in the Rust program is \"rust_\".",
                                    ])

                            prompt.extend(["\n## Response format", "In summary, please respond in the following JSON format:"]) 
                            prompt.extend([functional_modify_template])
                        
                            mod_count += 1

                        prompt = []
                        ongoing_in_mode_flag = False
                        ongoing_flag = False
            

                if mode == 'read_data':
                    # if 'answer' in rsp_json:
                    #     code = rsp_json['answer']
                    #     append_file(execute_path, code)

                    if 'target_files' in rsp_json:
                        target_list = rsp_json['target_files']
                        if not isinstance(target_list, list):
                            target_list = [target_list]
                        sum_target_list.extend(target_list)

                    if 'file_slices' in rsp_json and rsp_json['file_slices'] is not None:
                        slice_list = rsp_json['file_slices']
                        if not isinstance(slice_list, list):
                            slice_list = [slice_list]
                        sum_slice_list.extend(slice_list)

                if mode == 'execute_command':
                    if 'answer' in rsp_json:
                        code = rsp_json['answer']
                        append_file(execute_path, code)
        

            if ongoing_in_mode_flag is False:
                break

            print("Keep going to receive Rust code in modifying.")
                        
            prompt = [f"Please continue with the JSON data response to resolve the error in test{test_number}."] 

            prompt.extend(["## Response rules:", 
                        f"- To avoid hitting the token limit, keep the JSON data included in one response within {output_max} tokens.", 
                        "- If the JSON data for a single mode response is likely to exceed the token limit, split the response into multiple parts.",
                        "- If the JSON data is the last part, set the `ongoing_in_mode` key to `False`. If there are remaining JSON data parts, set the `ongoing_in_mode` key to `True`.",
                        "- Each response should always be in a single mode (`read_data`, `modify_data`, `execute_command`), and `ongoing_in_mode` should only be used when further interaction is needed within that mode.",
                        "- If you want to switch modes, end the current mode by setting ongoing_in_mode to false",
                        "- The `modified_data` content in `modify_data` mode must be directly executable without any omissions.",                        
                        ])
                
            rsp_json = ask_llm(prompt, "continue", llm_interface)

        ######################## Advance file by file ########################

        print(f"Running program for the mode: {mode}")
        if mode == 'modify_data':
            print(f"In mode: {mode}")
            part_editied_files = reflect_line_modification(sum_modified_list, rust_io_dir, database_dir)
            #modified_c_keys = update_modified_keys(modified_c_keys, meta_dir, rust_c_map, part_editied_files)
            editied_files.extend(part_editied_files)

            #if not reflect_success:
            #    return repair_count, error

        elif mode == 'read_data':
            print(f"In mode: {mode}")
            #output = run_read_script(execute_path, 20, True, None, "both")
            read_prompt = ["\n## Read data result", "The content obtained in read_data mode is as follows.", ""] 

            for see_path in sum_target_list:
                file_code = get_lined_code(see_path, mix_io_dir)
                read_prompt.extend([f"- Content of the file {see_path}:"]) 
                file_code = trim_code(see_path, file_code, 100000)
                read_prompt.extend([f'{file_code}\n'])

            for see_item in sum_slice_list:
                file_code = get_lined_specific_code(database_dir, see_item['file_path'], see_item['start_line'], see_item['end_line'], mix_io_dir)
                read_prompt.extend([f"- Content of {see_item['start_line']} - {see_item['end_line']} lines in the file {see_item['file_path']}:"])
                read_prompt.extend([f'{file_code}\n'])

            print("End of rsp_json")
        
        elif mode == 'execute_command':
            #if ready_to_execute is False: # turned this off
            print(f"In mode: {mode}")
            execute_error, execute_out = run_script_wo_log(execute_path, given_time, True, None, "both")
            #execute_out = run_script_pty(run_path, given_time)
            execute_out = run_script_pty(execute_path, given_time)

            print(f"Stop here: {execute_error}")
            print(f"Stop here: {execute_out}")
            
            """
            if process_type == "explore":
                is_covered = get_is_covered(entry, cov_detail_path, target_dir, cov_dir) # target_lineがカバーされているかどうかを検知する
            """

        elif mode == 'gdb_execute':
            
            print(f"In mode: {mode}")
            breakpoints = None
            use_breakpoints = False
        
            if 'breakpoints' in rsp_json:
                breakpoints = rsp_json['breakpoints']
            if 'use_breakpoints' in rsp_json:
                use_breakpoints = rsp_json['use_breakpoints']

            debug_with_pexpect(c_run_script, target_dir, breakpoints, use_breakpoints)

            """
            # gdb
            child_exp_data = {}
            child_repair_count = 1
            child_exp_data['repair_count'] = child_repair_count
            child_exp_data['experiment_path'] = "gdb_shell.c"
            child_exp_data['file_path'] = "gdb_shell.c"
            child_exp_data['average'] = 0
            
            run_gdb_path = f"{target_dir}/run_gdb.sh"
            create_permissioned_file(run_gdb_path)

            interface = {
                "run_gdb_path" : run_gdb_path,
                "run_test_path" : run_test_path, #run_gdb_path,
                "run_path" : run_path,
                "target_dir" : None, 
                "meta_dir" : None,
                "dep_json_path" : dep_json_path,
                "exp_data" : exp_data,
                "repair_count" : child_repair_count,
                "execute_path" : execute_path,
                "target_dir" : target_dir,
                "test_path" : None,
                "file_path" : None,
                "test_id" : None,
                "name" : None,
                "main_flag" : None,
                "cmd_list" : cmd_list,
            }
            repair_execute("gdb", interface)  # gdb_out_path = 
            gdb_done = True

            # Valgrind and Callgrind
            child_exp_data = {}
            child_repair_count = 1
            child_exp_data['repair_count'] = child_repair_count
            child_exp_data['experiment_path'] = "val_shell.c"
            child_exp_data['file_path'] = "val_shell.c"
            child_exp_data['average'] = 0
            
            run_val_path = f"{target_dir}/run_val.sh"
            create_permissioned_file(run_val_path)

            interface = {
                "run_val_path" : run_val_path,
                "run_test_path" : run_test_path, #run_val_path,
                "run_path" : run_path,
                "target_dir" : None, 
                "meta_dir" : None,
                "dep_json_path" : dep_json_path,
                "exp_data" : exp_data,
                "repair_count" : child_repair_count,
                "execute_path" : execute_path,
                "target_dir" : target_dir,
                "test_path" : None,
                "file_path" : None,
                "test_id" : None,
                "name" : None,
                "main_flag" : None,
                "cmd_list" : cmd_list,
            }
            repair_execute("val", interface) # val_out_path = 
            val_done = True
            """

        """
        elif mode == 'delete_data':
            print(f"In mode: {mode}")
            reflect_line_deletion(sum_deleted_list, rust_io_dir) # execute_error =  #sum_modified_list.extend(added_list) #if MOD_LINE:
        """

        repair_count += 1

    iteration_dict[repair_target] = repair_count

    summary_dict = {}
    if os.path.exists(f"{database_dir}/inner_repair.json"):
        summary_dict = read_json(f"{database_dir}/inner_repair.json")
    
    global called_count
    summary_dict[str(called_count)] = repair_count
    write_json(f"{database_dir}/inner_repair.json", summary_dict)

    called_count += 1

    return repair_count, modified_c_keys



def get_equivalent(modifications):
    print("Getting equivalency...")

    if not isinstance(modifications, list):
        modifications = [modifications]

    is_equivalent = True
    for item in modifications:
        if 'is_JSON' in item and item['is_JSON'] is True: # If modified_data is a list or dict, convert it to a string with json.dumps and then load it
            if isinstance(item['modified_data'], (list, dict)):
                json_content = item['modified_data']  # Loads are not necessary as it is already a Python object
            else:
                json_content = json.loads(item['modified_data'])  # loads for strings
            
            for entry in json_content:
                if 'equivalent' in entry:
                    if entry['equivalent'] is False:
                        is_equivalent = False

    return is_equivalent


def reformat_flow(repair_target, interface): 

    mix_io_dir = interface['mix_io_dir']
    exp_data = interface['exp_data']
    c_io_dir = interface['c_io_dir']
    rust_io_dir = interface['rust_io_dir']
    repair_count = interface['repair_count']
    llm_interface = interface['llm_interface']
    
    if repair_target == "get_analysis":
        execute_path = f"{mix_io_dir}/execute.sh"
        execute_dir = os.path.dirname(os.path.normpath(execute_path))

        c_flow = interface['c_flow']
        rust_flow = interface['rust_flow']
        flow_path = interface['flow_path']

    if repair_target != "get_analysis":
        run_path = run_all_path
        print(f"run_path is {run_path}")

    execute_path = f"{mix_io_dir}/execute.sh"
    if not os.path.exists(execute_path):
        create_permissioned_file(execute_path)

    execute_dir = os.path.dirname(os.path.normpath(execute_path))


    mode = None
    execute_error = None
    execute_out = None
    read_prompt = None

    error = True # Assume there is an error
    ongoing_flag = True #False
    mode = None

    modified_files = set()
    receive_count = 0
    is_equivalent = True
    equivalent_count = 0

    while (1):

        if exp_data['repair_count'] == REPAIR_MAX:
            print(f"Force to finish. Hit the REPAIR_MAX ({REPAIR_MAX}).")
            iteration_dict[repair_target] = repair_count
            sys.exit(1)
        
        print(f"Judging at {repair_count}: mode: {mode}, ongoing_flag: {ongoing_flag}, error: {error}")
        if (mode != "read_data" and ongoing_flag is False) or is_equivalent is False:  # error is None and 
            break
            
        if repair_target == "get_analysis":
            if repair_count == 1:
                prompt = []
                prompt.extend(["Below is information about the function flow from executing testcases of a C program and its Rust-converted counterpart.",
                            "Assuming the C execution results are the correct ideal values, determine the corresponding Rust function names, compare them with the converted Rust execution flow, and create JSON data summarizing whether the argument and return values match for every C function call (call_order).",
                            "Please follow the response rules and response rules below when answering.",
                            "",
                            "## Response rules:",
                            "- The Rust program executes by calling the Rust main function, rust_main(), from the C program's main function via FFI.",
                            f"- Please respond with JSON data in the following format to {flow_path}, using the modify_data mode.",
                            f"- This time, since we will keep updating {flow_path}, please set the \"start_line\" to 1 and \"end_line\" to -1 for the modification target in {flow_path} in modify_data mode.",    
                            f"- Please write the C function call order in \"call_order\".",
                            "- Not all C functions are converted 1-to-1 to Rust functions, so if there is no corresponding Rust function for a given C call_order, set \"found_correspondence\" to False and set the \"function_name\" value inside \"rust_call\" to None. Also, set \"equivalent\" to True.",
                            "- For \"equivalent\", judge whether the values of each argument and return variable match between the C and Rust call_order, and write True if they match, or False otherwise.",
                            #f"- To avoid exceeding the token limit of the output, limit the JSON data in a single response to {output_max} tokens or less. If necessary, provide the answer in multiple responses, but for now, respond with only the first portion of the JSON data with a clear delimiter.",
                            #f"- When responding in multiple parts, if there is more JSON data remaining, set the value of the 'ongoing' key, on the same level as the 'answer' key, to a boolean value of True. If the JSON data is the final portion, set the 'ongoing' key to a boolean value of False.",      
                            f"- To avoid hitting the output token limit, please keep the JSON data in a single response to {output_max} tokens or less.", # For long responses,
                            #f"When responding in multiple parts, if there is more JSON data remaining, set the value of the 'ongoing' key to a boolean True. If the JSON data is the final portion, set the 'ongoing' key to a boolean False.",  
                            #"- The modified code will be directly copy-pasted and executed, so absolutely do not include any omitted sections. If the JSON data in a single response is about to exceed the token limit, please split it across multiple responses.",
                            #"- If the JSON data is the final portion, set the 'ongoing' key to a boolean False. If there is more JSON data remaining, set the 'ongoing' key to a boolean True.",
                            "- If the JSON data in a single mode response is about to exceed the token limit, please split it across multiple responses.",
                            "- If the JSON data is the final portion, set the 'ongoing_in_mode' key to a boolean False. If there is more JSON data remaining, set the 'ongoing_in_mode' key to a boolean True.",
                            "- Always create a response using a single mode (`read_data`, `modify_data`, `execute_command`), and use `ongoing_in_mode` only when further exchanges are needed within that single mode.",
                            #"- When switching modes, end the response by requesting a new request.",
                            "- The content of modified_code in `modify_data` mode responses will be directly copy-pasted, so absolutely do not include any omitted sections.",  
                            ])

                prompt.extend(["", f"## Content to write to {flow_path}:"])
                prompt.extend([compare_template])


                prompt.extend([f"", "## Original correct C execution flow:"])
                prompt.extend([c_flow])

                prompt.extend([f"", "## Converted Rust execution flow:"])
                prompt.extend([rust_flow])
            


                prompt.extend([#"To resolve this, select exactly one of the following three modes and generate a response.", #The response must select exactly one mode.",
                        "",
                        "## Response types:",
                        "1. For 'read_data' mode:",
                        "## Purpose:",
                        #"- The execution result of the shell script code you responded with will be sent back to you, so you can see it.", #(cat, less, tree etc...)
                        "- The contents of the specified file name will be sent back as-is.",
                        "## How to respond:",
                        #"- Place the shell script code to execute inside the \"answer\" key of the JSON data.",
                        "- Place the file path you want to see inside the \"target_files\" key of the JSON data.",
                        #f"- The shell script code you respond with will be saved to the shell script file at {execute_path} and will be executed in the directory where {execute_path} is located.",
                        #"- If you want to see multiple files, write multiple commands in a single shell script code response.",
                        #f"- The shell script can contain multiple commands.",
                        #"- For example, if the code contains cat /path/to/test.c, the contents of /path/to/test.c will be sent back to you in the subsequent request prompt.",
                        "",
                        "2. For 'modify_data' mode:",
                        "## Purpose:",
                        "- You can modify existing files at specified file paths.",
                        "## How to respond:",
                        f"- Delete the original code and insert the file name, start line, and end line of the section to be changed into the \"file_path\", \"start_line\", and \"end_line\" key values of the JSON data respectively.",
                        "- Then, place the new content to be inserted at that modification point into the \"modified_code\" key value.",
                        "- modified_code will be directly copy-pasted into the original code from start_line to end_line, so please include appropriate indentation so it executes correctly when copy-pasted.",
                        "- modified_code will be directly copy-pasted into the original code, so absolutely do not include any omitted sections.",
                        "- If you want to only delete without inserting at a specific location in the specified file path, set the \"is_deletion\" value to True.",
                        "- If you want to overwrite the entire file rather than modifying only the specified line range, set the \"overwrite_all\" value to True.",
                        "- If you are not sure about the exact start_line and end_line, executing 'read_data' mode first will show the file's code contents with line numbers.",
                        "",
                        "3. For 'execute_command' mode:",
                        "## Purpose:",
                        "- You can execute the shell script code you respond with.",
                        "## How to respond:",
                        ])
                if repair_target != "get_analysis":
                    prompt.extend([f"- This runs separately from {run_path}. If it is not needed apart from {run_path}, you do not need to respond with this."])

                prompt.extend(["- Place the shell script code to execute inside the \"answer\" key of the JSON data.",
                            f"- The shell script code you respond with will be saved to the shell script file at {execute_path} and executed in the {execute_dir} directory.",
                            f"- Please ensure that ./execute.sh is executed without arguments.",
                            f"- The shell script can contain multiple commands.",
                            ])
            
                        
                
                prompt.extend(["\n## Response format", f"In summary, please respond in the following JSON format:"])
                prompt.extend([autonomous_template])

                
            else:
                prompt = []
                prompt.extend([#"There is information about function flows from executing testcases of a C program and its Rust-converted counterpart.",
                            "Please continue responding with JSON data summarizing whether the argument and return values match between C and Rust calls for all C function calls (call_order).",
                            "Please follow the response rules and response rules when answering.",
                            ])
                
            receive_count += 1  # For some reason it seems better to have it here  # Since run_script is not called, it's slightly different
              

        if execute_error is not None or execute_out is not None:
            if execute_out is not None:
                prompt.extend(["\n## Execution result:",
                            "The result executed in execute_command mode is as follows:"
                            f"{execute_out}",
                            ""])

            if execute_error is not None:
                prompt.extend(["\n## Execution result error: ",
                            "The result error executed in execute_command mode is as follows:"
                            f"{execute_error}"])
            execute_error = None # Initialize
            execute_out = None
        
        if read_prompt is not None:
            prompt.extend(["", "Previous response:"])
            prompt.extend(read_prompt)
            read_prompt = None # Initialize

        print(f"ongoing_flag is {ongoing_flag}")

        if ongoing_flag is False:
            prompt.extend([#"To resolve this, select exactly one of the following three modes and generate a response.", #The response must select exactly one mode.",
                        "",
                        "## Response types:",
                        "1. For 'read_data' mode:",
                        "## Purpose:",
                        #"- The execution result of the shell script code you responded with will be sent back to you, so you can see it.", #(cat, less, tree etc...)
                        "- The contents of the specified file name will be sent back as-is.",
                        "## How to respond:",
                        #"- Place the shell script code to execute inside the \"answer\" key of the JSON data.",
                        "- Place the file path you want to see inside the \"target_files\" key of the JSON data.",
                        #f"- The shell script code you respond with will be saved to the shell script file at {execute_path} and will be executed in the directory where {execute_path} is located.",
                        #"- If you want to see multiple files, write multiple commands in a single shell script code response.",
                        #f"- The shell script can contain multiple commands.",
                        #"- For example, if the code contains cat /path/to/test.c, the contents of /path/to/test.c will be sent back to you in the subsequent request prompt.",
                        "",
                        "2. For 'modify_data' mode:",
                        "## Purpose:",
                        "- You can modify existing files at specified file paths.",
                        "## How to respond:",
                        f"- Delete the original code and insert the file name, start line, and end line of the section to be changed into the \"file_path\", \"start_line\", and \"end_line\" key values of the JSON data respectively.",
                        "- Then, place the new content to be inserted at that modification point into the \"modified_code\" key value.",
                        "- modified_code will be directly copy-pasted into the original code from start_line to end_line, so please include appropriate indentation so it executes correctly when copy-pasted.",
                        "- modified_code will be directly copy-pasted into the original code, so absolutely do not include any omitted sections.",
                        "- If you want to only delete without inserting at a specific location in the specified file path, set the \"is_deletion\" value to True.",
                        "- If you want to overwrite the entire file rather than modifying only the specified line range, set the \"overwrite_all\" value to True.",
                        "- If you are not sure about the exact start_line and end_line, executing 'read_data' mode first will show the file's code contents with line numbers.",
                        "",
                        "3. For 'execute_command' mode:",
                        "## Purpose:",
                        "- You can execute the shell script code you respond with.",
                        "## How to respond:",
                        ])

            if repair_target != "get_analysis":
                prompt.extend([f"- This runs separately from {run_path}. If it is not needed apart from {run_path}, you do not need to respond with this."])

            prompt.extend(["- Place the shell script code to execute inside the \"answer\" key of the JSON data.",
                        f"- The shell script code you respond with will be saved to the shell script file at {execute_path} and executed in the {execute_dir} directory.",
                        f"- Please ensure that ./execute.sh is executed without arguments.",
                        f"- The shell script can contain multiple commands.",
                        ])
        
            prompt.extend(["\n## Response format", f"In summary, please respond in the following JSON format:"])
            prompt.extend([autonomous_template])

            # if error is not None and error is not True:
            #     prompt.extend(["", "## Error:", error])

            if error is not None and error is not True:
                prompt.extend(["", f"## Error from {run_path}:", f"{error}"]) # error])
                #prompt.extend(error)

                prompt.extend([f"\n## Execution result of {run_path}:", f"{std_out}"]) #, std_out]) #, std_out]) # std_out])
                #prompt.extend(std_out)
                std_out = ""

        prompt = adjust_prompt(prompt)
        print("-------------------------")

        print(f"repair_target: {repair_target}")
        exp_data['repair_count'] =  repair_count

        if repair_count == 1:
            rsp_json = ask_llm(prompt, "init", llm_interface)
        else:
            delete_file(execute_path)
            create_permissioned_file(execute_path)
            rsp_json = ask_llm(prompt, "continue", llm_interface)

        #ongoing_flag = False
        ongoing_in_mode_flag = False

        sum_target_list = []
        sum_modified_list = []
        sum_deleted_list = []

        while (1):
            execute_error = None
            if 'mode' in rsp_json:
                mode = rsp_json['mode']

                if mode == 'read_data':
                    if 'answer' in rsp_json:
                        code = rsp_json['answer']
                        append_file(execute_path, code)

                    if 'target_files' in rsp_json:
                        target_list = rsp_json['target_files']
                        if not isinstance(target_list, list):
                            target_list = [target_list]
                        sum_target_list.extend(target_list)

                if mode == 'modify_data':
                    if 'answer' in rsp_json:
                        modified_list = rsp_json['answer'] # Maybe we could put individually converted items here
                        if not isinstance(modified_list, list):
                            modified_list = [modified_list]
                        sum_modified_list.extend(modified_list)

                        # It might be faster to find false here without waiting for sum_modified_list
                        is_equivalent = get_equivalent(modified_list)
                        if is_equivalent is False:
                            equivalent_count += 1
                        
                        if equivalent_count < 2:
                            is_equivalent is True

                if mode == 'execute_command':
                    if 'answer' in rsp_json:
                        code = rsp_json['answer']
                        append_file(execute_path, code)
        

            print(f"ongoing_flag is {ongoing_flag}")
            if 'ongoing' in rsp_json:
                ongoing_flag = rsp_json['ongoing']

            if 'ongoing_in_mode' in rsp_json:
                ongoing_in_mode_flag = rsp_json['ongoing_in_mode']

            if ongoing_in_mode_flag is False:
                break

            if is_equivalent is False:  # Distinctive
                break

            print("Keep going to receive Rust code in modifying.")
            
            prompt = []
            prompt.extend(["Please continue responding with JSON data summarizing whether the argument and return values match between C and Rust calls for all C function calls (call_order).",
                          "Please follow the response rules and response rules when answering."])

            prompt.extend(["", "## Response rules:",
                        "- If there is an error when representing a backslash as a byte literal, you need to escape the backslash in the source code and also escape it within the byte literal, so please use three backslashes (double backslash).",
                        "- If there is an error when representing a backslash as a character literal, you need to escape the backslash in the source code and also escape it within the character literal, so please use two backslashes.",
                        f"- To avoid hitting the output token limit, please keep the JSON data in a single response to {output_max} tokens or less.", # For long responses,
                        #f"When responding in multiple parts, if there is more JSON data remaining, set the 'ongoing' key to a boolean True. If the JSON data is the final portion, set the 'ongoing' key to a boolean False.",  
                        #"- The modified code will be directly copy-pasted and executed, so absolutely do not include any omitted sections. If the JSON data in a single response is about to exceed the token limit, please split it across multiple responses.",
                        #"- If the JSON data is the final portion, set the 'ongoing' key to a boolean False. If there is more JSON data remaining, set the 'ongoing' key to a boolean True.",
                        "- If the JSON data in a single mode response is about to exceed the token limit, please split it across multiple responses.",
                        "- If the JSON data is the final portion, set the 'ongoing_in_mode' key to a boolean False. If there is more JSON data remaining, set the 'ongoing_in_mode' key to a boolean True.",
                        "- Always create a response using a single mode (`read_data`, `modify_data`, `execute_command`), and use `ongoing_in_mode` only when further exchanges are needed within that single mode.",
                        #"- When switching modes, end the response by requesting a new request.",
                        "- The content of modified_code in `modify_data` mode responses will be directly copy-pasted, so absolutely do not include any omitted sections.",
                        ])

            rsp_json = ask_llm(prompt, "continue", llm_interface) # code_blocks = extract_code_blocks(response)

        ######################## Proceed per file ########################

        print(f"Running program for the mode: {mode}")
        if mode == 'modify_data':
            print(f"In mode: {mode}")
            print(f"sum_modified_list at reformate_flow: {sum_modified_list}")
            reflect_line_modification(sum_modified_list, rust_io_dir, database_dir)

        elif mode == 'read_data':
            print(f"In mode: {mode}")
            #output = run_read_script(execute_path, given_time, True, None, "both")
            read_prompt = ["- The information obtained via read_data mode is as follows.", ""] #Even if you had a response with the 'ongoing' flag set to true before this, please make sure to respond once with None in the \"answer\" key of the JSON data for this reply.", 
            
            for see_path in sum_target_list:
                file_code = get_lined_code(see_path, mix_io_dir)
                read_prompt.extend([f'Contents of the file {see_path}:'])
                read_prompt.extend([f'{file_code}\n'])

            print("End of rsp_json")
        
        elif mode == 'execute_command':
            print(f"In mode: {mode}")
            execute_error, execute_out = run_script(execute_path, given_time, True, None, "both")
            #execute_out = run_script_pty(run_path, given_time)
            execute_out = run_script_pty(execute_path, given_time)
            
        repair_count += 1

    # Put this on hold for now
    # check_dif(target_dir)

    iteration_dict[repair_target] = repair_count


compare_template = f"""
[
    {{
        "call_order": 1,
        "c_call": {{
            "function_name": (function name),
            "file_path": (file containing the call),
            "line_number" : (line number within file_path),
            "args": {{
                "(argument name 1)": (value of argument 1),
                "(argument name 2)": (value of argument 2),
                ...
            }},
            "return_value": {{
                "(return value variable name)": (value of the return value),
            }}
            "in_out": IN or OUT
        }},
        "rust_call": {{
            "function_name": (function name),
            "file_path": (file containing the call),
            "line_number" : (line number within file_path),
            "args": {{
                "(argument name 1)": (value of argument 1),
                "(argument name 2)": (value of argument 2),
                ...
            }},
            "return_value": {{
                "(return value variable name)": (value of the return value),
            }}
            "in_out": IN or OUT,
            "found_correspondence" : True or False, 
        }},
        "equivalent": True or False,
        "reason" : (reason or explanatory text for why "equivalent" was judged as True or False)
    }},
    {{ 
        "call_order": 2,
        ...
    }}, ...
]
"""

def print_function_flow(language_key, call_data, pre_lange, post_lange):
    print(f"\nFunction Call Flow for {language_key.upper()}:")
    result = []
    current_depth = 0
    
    target_count = 0
    i = 0
    for call in call_data:
        i += 1
        if call["equivalent"] is False:
            target_count = i        

    print(f"target_count: {target_count}")
    print(f"target_count - pre_lange: {target_count - pre_lange}")
    print(f"target_count + post_lange: {target_count + post_lange}")

    i = 0
    for call in call_data:
        i += 1

        call_info = call[f"{language_key}_call"]
        if not call_info:
            continue

        if not(target_count - pre_lange <= i and i <= target_count + post_lange):
            continue
        
        # Calculate indentation
        indent = "    " * call_info.get("depth", current_depth)
        
        # Print arrow for function entry/exit
        arrow = "➤" if call_info["in_out"] == "IN" else "←"
        
        # Get function information
        func_name = call_info["function_name"]
        file_path = call_info["file_path"]
        line_number = call_info["line_number"]
        
        # Format arguments and return value
        args_str = format_args(call_info.get("args", {}))
        return_str = format_args(call_info.get("return_value", {}))
        
        # Add difference marker for non-equivalent calls
        diff_marker = ""
        if language_key == "rust":
            if not call["equivalent"]:
                #diff_marker = " [DIFFERENT]" if not call["equivalent"] else ""
                diff_marker = " ⚠ NON-EQUIVALENT CALL ⚠ "

        # Construct the output line
        if call_info["in_out"] == "IN":
            print(f"{indent}{arrow} {func_name} @ {file_path}:{line_number}{diff_marker}")
            print(f"{indent}    IN  → {args_str}")

            result.append(f"{indent}{arrow} {func_name} @ {file_path}:{line_number}{diff_marker}")
            result.append(f"{indent}    IN  → {args_str}")

        else:
            print(f"{indent}    OUT {arrow} {return_str}")
            result.append(f"{indent}    OUT {arrow} {return_str}")
    
    return result 



def get_tree_flow_not(language_key, flow_path):
    call_data = read_json(flow_path)

    print(f"\nFunction Call Flow for {language_key.upper()}:")
    result = {}
    current_depth = 0
    

    i = 0
    for call_info in call_data:
        test_name = call_info['test_name']
        if test_name not in result:
            result[test_name] = []
        i += 1

        if "input" in call_info:
            call_info['call_type'] = "input"
        
        if "output" in call_info:
            call_info['call_type'] = "output"
        
        # Calculate indentation
        indent = "    " * call_info.get("depth", current_depth)
        
        # Print arrow for function entry/exit
        arrow = "➤" if call_info["call_type"] == "input" else "←"
        
        # Get function information
        func_name = call_info["name"]
        file_path = call_info["file_path"]
        line_number = call_info["def_start_line"]
        #def_start_line = call_info["def_start_line"]
        #line_number = call_info["line_number"]
        
        # Format arguments and return value
        args_str = format_args(call_info.get("arguments", {}))
        return_str = format_args(call_info.get("return_value", {}))
        
        # Add difference marker for non-equivalent calls
        diff_marker = ""

        # Construct the output line
        if call_info["call_type"] == "input":
            print(f"{indent}{arrow} {func_name} @ {file_path}:{line_number}{diff_marker}")
            print(f"{indent}    IN  → {args_str}")

            result[test_name].append(f"{indent}{arrow} {func_name} @ {file_path}:{line_number}{diff_marker}")
            result[test_name].append(f"{indent}    IN  → {args_str}")

        else:
            print(f"{indent}    OUT {arrow} {return_str}")
            result[test_name].append(f"{indent}    OUT {arrow} {return_str}")
    
    return result 


def scope_flow(flow_path):
    if not os.path.exists(flow_path):
        return [], []

    flow_data = read_json(flow_path)
    pre_lange = 3
    post_lange = 3

    # Print flow for C code
    c_flow = print_function_flow("c", flow_data, pre_lange, post_lange)
    
    # Print flow for Rust code
    rust_flow = print_function_flow("rust", flow_data, pre_lange, post_lange)

    return c_flow, rust_flow

# It would be better to narrow it down to only the incorrect test cases
def get_flow_data(rust_log_path, rust_flow_path, golden_flow_path):

    print("Getting flow data at get_flow_data()")

    rust_parse_log(rust_log_path, rust_flow_path)
    rust_flow, file_data = show_flow(rust_flow_path)

    os.makedirs(f"{mix_io_dir}/flows", exist_ok=True)

    for test_name, line_data in file_data.items():
        rust_flow_data = ""

        output_file = f"{mix_io_dir}/flows/{test_name}.txt"

        golden_path = f"golden/{test_name}_golden_flow.txt"
        golden_flow = read_file(golden_path)

        # Write comparison to file
        with open(output_file, 'w', encoding='utf-8') as f:
            # Write C program section
            f.write("[Expected correct flow of initial C program]\n")
            f.write(golden_flow)
            f.write("\n")  # Add separator
            
            # Write Rust program section
            f.write("[Current flow of translated Rust Program]\n")
            f.write(f"{test_name}" + '\n')
            for line in line_data:
                f.write(str(line) + '\n')
                rust_flow_data += str(line) + '\n'

        os.makedirs(f"{mix_io_dir}/analysis", exist_ok=True)

        flow_path = f"{mix_io_dir}/analysis/{test_name}.json"
    
        exp_data = {}
        exp_data['experiment_path'] = "reformat.c"
        exp_data['file_path'] = "reformat.c" 
        exp_data['repair_count'] = 0
        exp_data['average'] = 0
        

        repair_count = 1
        interface = {
            "c_flow" : golden_flow,
            "rust_flow" : rust_flow_data,
            "flow_path" : flow_path,
            "c_io_dir" : c_io_dir,
            "rust_io_dir" : rust_io_dir,
            "mix_io_dir" : mix_io_dir,
            "exp_data" : exp_data,
            "repair_count" : repair_count,

        }

    # Put spots for incorrect parts in the answers directory.
    for test_name, line_data in file_data.items():
        flow_path = f"{mix_io_dir}/analysis/{test_name}.json"

        c_flow, rust_flow = scope_flow(flow_path)
        output_file = f"{mix_io_dir}/flows/{test_name}.txt"

        # Write comparison to file
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write("[Expected correct flow of initial C program]\n")
            for line in c_flow:
                f.write(str(line) + '\n')
            
            f.write("[Current flow of translated Rust Program]\n")
            f.write(f"{test_name}" + '\n')
            for line in rust_flow:
                f.write(str(line) + '\n')


def compare_io(rust_log_path, c_flow_path, rust_flow_path, o_meta_dir, golden_flow_path):

    rust_parse_log(rust_log_path, rust_flow_path)

    result = {}
    result['entire'] = False

    moment_c_flows = []
    moment_c_flows = read_json(c_flow_path)

    moment_rust_flows = []
    moment_rust_flows = read_json(rust_flow_path)

    golden_flows = []
    golden_flows = read_json(golden_flow_path) 

    """
    # rust_flow_path probably contains a mixture of C and Rust output
    for item in rust_flow:
        if item['lang'] == "C":
            moment_c_flows.append(item)
        
        elif item['lang'] == "Rust":
            moment_rust_flows.append(item)
    """
    # Check whether the final output is the same
    all_success = True
    for item in moment_c_flows:
        for glod_item in golden_flows:
            if (glod_item['file_path'] == item['file_path']
                and glod_item['def_start_line'] == item['def_start_line']
                and glod_item['name'] == item['name']):
                
                item['result'] = "success"
            else:
                item['result'] = "fail"
                all_success = False

            print(f"- Result for {item['name']} at line {item['def_start_line']} in {item['file_path']}: {item['result']}")
            print(f"      C side: {glod_item['file_path']} <-> Rust side: {item['file_path']}")

    # pending
    # On the other hand, also look at intermediate progress (for this, correspondence needs to be checked)
    for item in moment_rust_flows:
        file_path = item['file_path']

    return moment_c_flows, moment_rust_flows


def get_fail_flow_old(test_report, test_number, rust_log_path, rust_flow_path, golden_flow_path):
    print("Getting flow data")

    fail_numbers = []
    fail_numbers.append(f"test{str(test_number)}")

    rust_parse_log(rust_log_path, rust_flow_path)
    if not os.path.exists(rust_flow_path):
        return

    file_data = get_tree_flow("Rust", rust_flow_path)

    os.makedirs(f"{mix_io_dir}/flows", exist_ok=True)
    os.makedirs(f"{mix_io_dir}/analysis", exist_ok=True)
    os.makedirs(f"{mix_io_dir}/scope", exist_ok=True)


    for test_name, line_data in file_data.items():
        if test_name not in fail_numbers:
            continue

        rust_flow_data = ""
        golden_flow = ""

        output_file = f"{mix_io_dir}/flows/{test_name}.txt"

        golden_path = f"golden/{test_name}_golden_flow.txt"
        golden_flow = read_file(golden_path)

        # Write comparison to file
        with open(output_file, 'w', encoding='utf-8') as f:
            # Write C program section
            f.write("[Expected correct flow of initial C program]\n")
            if golden_flow is not None:
                f.write(golden_flow)
                f.write("\n")  # Add separator
            
            # Write Rust program section
            f.write("[Current flow of translated Rust Program]\n")
            f.write(f"{test_name}" + '\n')
            for line in line_data:
                f.write(str(line) + '\n')
                rust_flow_data += str(line) + '\n'

        flow_path = f"{mix_io_dir}/analysis/{test_name}.json"
    
        exp_data = {}
        exp_data['experiment_path'] = "reformat.c"
        exp_data['file_path'] = "reformat.c" 
        exp_data['repair_count'] = 0
        exp_data['average'] = 0
        

        repair_count = 1
        interface = {
            "c_flow" : golden_flow,
            "rust_flow" : rust_flow_data,
            "flow_path" : flow_path,
            "c_io_dir" : c_io_dir,
            "rust_io_dir" : rust_io_dir,
            "mix_io_dir" : mix_io_dir,
            "exp_data" : exp_data,
            "repair_count" : repair_count,

        }

        # temporarily put on hold
        #reformat_flow("get_analysis", interface)  ##repair_execute("get_analysis", interface)


def find_rust_log_path(work_dir, base_name):
    """Search under work_dir to find the trace log file matching base_name"""
    for root, dirs, files in os.walk(work_dir):
        candidate = os.path.join(root, base_name)
        if os.path.exists(candidate):
            full_path = os.path.abspath(candidate)
            log_dir = os.path.dirname(full_path)
            return full_path, log_dir
    
    return None, None


def get_fail_flow(work_dir, test_number): 
    print("Getting flow data")

    test_number = str(test_number)

    base_name = f"flow_results/test{test_number}_trace.log"

    rust_log_path, rust_log_dir = find_rust_log_path(work_dir, base_name)
    if rust_log_path is None:
        return None

    rust_flow_path = f"{rust_log_dir}/test{test_number}_flow.txt"

    parse_trace(rust_log_path, None, rust_flow_path, True) 

    return rust_flow_path


def clear_rust_flow(rust_log_path, rust_flow_path):
    delete_file(rust_log_path)
    delete_file(rust_flow_path)

    print("Cleared rust_flows")


def get_given_num(c_io_dir):

    flow_dir = f"{c_io_dir}/flow_results"
    
    if not os.path.exists(flow_dir):
        return 0
    
    test_numbers = []
    for filename in os.listdir(flow_dir):
        #match = re.match(r'test(\d+)_(success|fail)\.log', filename)
        match = re.match(r'test(\d+)_trace\.log', filename)

        if match:
            test_numbers.append(int(match.group(1)))
    
    if test_numbers:
        return max(test_numbers)
    
    raise ValueError("Did not find any test_number.")


def get_given_num_original(run_test_path):

    print(f"For {run_test_path}...")
    
    test_numbers = []
    
    with open(run_test_path, 'r', encoding='utf-8') as f:
        content = f.read()
        
    # Find all "Test X started" patterns
    started_pattern = r'echo\s+"Test\s+(\d+)\s+started"'
    ended_pattern = r'echo\s+"Test\s+(\d+)\s+ended"'
    
    started_matches = re.findall(started_pattern, content)
    ended_matches = re.findall(ended_pattern, content)
    
    # Verify that started and ended matches are consistent
    if started_matches == ended_matches:
        test_numbers = [int(num) for num in started_matches]
        print(f"  Found {len(test_numbers)} test(s): {test_numbers}")
    else:
        print(f"  Warning: Mismatch between started ({started_matches}) and ended ({ended_matches}) markers")
        # Use the union of both
        all_nums = set(started_matches) | set(ended_matches)
        test_numbers = sorted([int(num) for num in all_nums])
        print(f"  Found test numbers: {test_numbers}")

    # Return the maximum test number
    if test_numbers:
        max_num = max(test_numbers)
        print(f"  Maximum test number: {max_num}")
        return max_num
    else:
        print(f"  No test numbers found")
        return 0


def delete_all_log(given_test_number, c_io_dir):

    for test_number in range(1, given_test_number+1):
        success_path = f"{c_io_dir}/flow_results/test{test_number}_success.log"
        fail_path = f"{c_io_dir}/flow_results/test{test_number}_fail.log"
        delete_file(success_path)
        delete_file(fail_path)


def get_smallest_fail_id(given_test_number, c_io_dir, error):

    test_number = None
    for test_id in range(1, given_test_number+1):
        success_path = f"{c_io_dir}/flow_results/test{str(test_id)}_success.log"
        fail_path = f"{c_io_dir}/flow_results/test{str(test_id)}_fail.log"

        if not os.path.exists(success_path): #os.path.exists(fail_path):
            test_number = test_id
            break

    if test_number is not None:
        return test_number

    if error is not None:
        test_number = 1
        return test_number

    return None


def check_semantics(mix_io_dir, build_path, rust_build_path, run_test_path, run_all_path, run_all_template_path, rust_io_dir, c_io_dir, 
                    raw_dir, meta_dir, work_dir, target_dir, rust_output_dir, database_dir, chat_dir, log_dir, token_path, execute_path,
                    dep_json_path, c_rust_path, rust_c_path, time_path, given_time, target, explore_time, notes,
                    llm_interface, progress_queue, max_iterations, flow_on
                    ): 

    print("Repairing functional errors")

    # change run_all.sh
    generate_run_all_path(run_all_path, run_all_template_path, target)

    # check semantics
    exp_data = {}
    exp_data['experiment_path'] = "whole.rs"
    exp_data['file_path'] = "whole.rs" 
    exp_data['repair_count'] = 0
    exp_data['average'] = 0

    repair_count = 1
    interface = {
        'convert_element': "semantics",
        'mix_io_dir' : mix_io_dir,
        'meta_dir': meta_dir,
        'dep_json_path': dep_json_path,
        'exp_data': exp_data,
        'repair_count': repair_count,
        'run_test_path' : run_test_path,
        'c_io_dir' : c_io_dir,
        'rust_io_dir' : rust_io_dir,
        'run_all_path' : run_all_path,
        'rust_build_path' : rust_build_path,
    }

    interface = SemConfig(
        mix_io_dir=mix_io_dir,
        c_io_dir=c_io_dir,
        rust_io_dir=rust_io_dir,
        build_path=build_path,
        rust_build_path=rust_build_path,
        run_test_path=run_test_path,
        run_all_path=run_all_path,
        rust_c_path=rust_c_path,
        c_rust_path=c_rust_path,
        raw_dir=raw_dir,
        select=False,
        flow_on=flow_on,
        llm_interface=llm_interface,
        target=target,
        target_dir=target_dir,
        chat_dir=chat_dir,
        database_dir=database_dir,
        cov_target="function",
        time_path=time_path,
        work_dir=work_dir,
        token_path=token_path,
        original_target_dir=None,
        meta_dir=meta_dir,
        dep_json_path=dep_json_path,
        exp_data=exp_data,
        repair_count=repair_count,
        execute_path=execute_path,
        test_path=None,
        file_path=None,
        test_id=None,
        function_name=None,
        main_flag=None,
        explore_time=explore_time,
        notes=notes,
        progress_queue=progress_queue,
        log_dir=log_dir,
        max_iterations=max_iterations
    )

    run_path = run_all_path
    print(f"run_path is {run_path}")

    given_test_number = get_given_num(c_io_dir)

    # Run first
    error = None
    std_out = None
    mode = None
    ongoing_flag = None
    part_modify_count = 1
    
    print("Crearing log paths")
    delete_all_log(given_test_number, c_io_dir)
    error, std_out = run_script_wo_log(run_all_path, given_time, True, None, "both") #, rust_log_path, golden_flow_path) # #run_script(run_path, given_time, True, None, "both")

    print("======= error =======")
    print(error)
    print("\n======= std_out =======")
    print(std_out)
    print("\n==============")

    check_count = 1 
    judge_count = 0

    test_number = get_smallest_fail_id(given_test_number, c_io_dir, error)

    while (1):
        if test_number is None:
            break

        success_path = f"{c_io_dir}/flow_results/test{test_number}_success.log" # success_path = f"results/test{test_number}_success.log"
        fail_path = f"{c_io_dir}/flow_results/test{test_number}_fail.log" # fail_path = f"results/test{test_number}_fail.log"
        
        interface.repair_count = 1

        ###############################
        ## Repair semantics
        ###############################

        print(f"Repairing for {test_number} at count {part_modify_count}...")
        # Difficult to separate living here
        error_log = None
        if os.path.exists(fail_path):
            # Fix specific testcase
            error_log = read_file(fail_path)
        else:
            # Correct compilation errors to generall
            print("Fixing general error")

        """
        if os.path.exists(success_path) is True: 
            break
        """

        # fail flow acquisition
        # if flow_on:
        rust_flow_path = get_fail_flow(work_dir, test_number)

        # ready for repair and repair
        interface.test_number = test_number
        interface.error_log = error_log 
        interface.error = error
        interface.std_out = std_out

        interface.repair_count = repair_count
        interface.flow_path = rust_flow_path

        modified_c_keys = set()
        repair_count, modified_c_keys = repair_semantics("semantics", interface)

        delete_all_log(given_test_number, c_io_dir)
        error, std_out, repair_count = run_script(run_path, 100, True, None, "both", None, repair_count, None, None, mode)
        judge_count += 1

        print(f"Judging at {repair_count}: run_path: {run_path} mode: {mode}, ongoing_flag: {ongoing_flag}, error: {error}")
        test_number = get_smallest_fail_id(given_test_number, c_io_dir, error)

        """
        modified_rust_lines = get_modified_rust_lines(modified_c_keys, c_rust_path, meta_dir)

        c_mod_files = []
        rust_mod_files = []

        ###############################
        ## Correspondence mapping
        ###############################

        sum_answer_data = {}
        grouped_c_keys = get_grouped_c_keys(modified_c_keys, 10)
        for c_key_json in grouped_c_keys:
            #prompt.extend(c_code)
            #prompt.extend(modified_lines) 
            
            repair_count = 1
            interface = {
                'convert_element': 'divided_type',
                #'c_path': mod_c_path,
                #'rust_path': mod_rust_path,
                'meta_dir': meta_dir,
                'dep_json_path': dep_json_path,
                #'div_start_line': div_start_line,
                'exp_data': exp_data,
                #'modified_files': modified_files,
                'target_dir': rust_output_dir,
                'raw_dir' : raw_dir,
                'rust_build_path' : rust_build_path,
                'label' : label,
                'repair_count' : repair_count,
                'answer_path' : answer_path
            }
            
            interface = CorConfig(
                modified_lines=modified_lines,
                key_json=c_key_json,
                rust_path=rust_path,
                raw_dir=raw_dir,
                select=False,
                #llm_choice=llm_choice,
                llm_interface=llm_interface,
                target_dir=target_dir,
                chat_dir=chat_dir,
                database_dir=database_dir,
                cov_target="function",
                time_path=time_path,
                work_dir=work_dir,
                token_path=token_path,
                original_target_dir=original_dir,
                build_path=build_path,
                run_test_path=run_test_path,
                run_all_path=run_all_path,
                #run_gdb_path=run_gdb_path,
                #run_val_path=run_val_path,
                meta_dir=meta_dir,
                dep_json_path=dep_json_path,
                exp_data=exp_data,
                repair_count=repair_count,
                execute_path=execute_path,
                # cmd_list=cmd_list,
                explore_time=explore_time,
                # cmd_exe=cmd_exe,
                notes=notes,
                progress_queue=progress_queue,
                log_dir=log_dir,
                max_iterations=max_iterations
            )

            delete_file(answer_path)

            print(f"------ Start asking about the correspondence of {label} ------")
            ask_correspondence("ask_correspondence", interface) #repair_execute("ask_correspondence", interface) #ask_correspondence("divided", interface)
            print(f"------ End of asking about the correspondence of {label} ------")

            # reverse answer_data
            #if label != "function":
            reverse_tmp(answer_path, mod_rust_path, label)

            answer_data = read_json(answer_path)
            merge_json(sum_answer_data, answer_data)

        # update c rust metadata
        #update_c_rust_metadata(sum_answer_data, c_rust_path, rust_c_path) # mod_files, 
        update_c_rust_metadata(rust_output_dir, meta_dir, database_dir, sum_answer_data, c_rust_path, rust_c_path)
        """

        part_modify_count += 1
        if part_modify_count > 50:
            break
        judge_count += 1

    print(f"Finished of functional_check, judge count: {judge_count}, given_test_number: {given_test_number}")


def sanitize_identifier(file_path):
    stem = os.path.splitext(os.path.basename(file_path))[0]
    return re.sub(r'[^a-zA-Z0-9_]', '_', stem)


def convert_cargo_toml_to_binary(toml_path: str, target_name: str) -> None:
    """
    Convert Cargo.toml from a library crate to a binary crate.
    
    Operations:
    1. Remove the [lib] section if it exists
    2. Add a [[bin]] section with name and path
    3. Remove crate-type if specified
    4. Preserve all other sections ([package], [dependencies], [build-dependencies], etc.)
    
    Args:
        toml_path: Path to Cargo.toml
        target_name: Binary name (set as the name field in [[bin]])
    """
    import tomlkit
    
    with open(toml_path, 'r') as f:
        doc = tomlkit.parse(f.read())
    
    # 1. Remove the [lib] section
    if 'lib' in doc:
        del doc['lib']
    
    # 3. Remove crate-type from [package] (if present)
    if 'package' in doc and 'crate-type' in doc['package']:
        del doc['package']['crate-type']
    
    # 2. Add the [[bin]] section
    bin_table = tomlkit.table()
    bin_table['name'] = target_name
    bin_table['path'] = 'src/main.rs'
    
    bin_array = tomlkit.aot()  # Array of Tables
    bin_array.append(bin_table)
    doc['bin'] = bin_array
    
    with open(toml_path, 'w') as f:
        f.write(tomlkit.dumps(doc))


reformat_response = f"""# In "modify_data" mode
{{
    "mode" : "modify_data",
    "answer" : [
        {{
            "file_path" : (file path),
            "start_line" : (start line of the original code to be deleted; must reflect the original range to be replaced),
            "end_line" : (end line of the original code to be deleted; must reflect the original range to be replaced),
            "modified_data" : (Content of the corrected code without any omission. Content of the corrected code as a string if is_JSON is false, or as a direct JSON object if is_JSON is true.),
        }},
        {{
            "file_path" : (file path),
            "start_line" : (start line of the original code to be deleted; must reflect the original range to be replaced),
            "end_line" : (end line of the original code to be deleted; must reflect the original range to be replaced),
            "modified_data" : (Content of the corrected code without any omission. Content of the corrected code as a string if is_JSON is false, or as a direct JSON object if is_JSON is true.),
        }},...
    ],
    "ongoing" : true if the response will continue in a different mode. false otherwise,
    "ready_to_execute" : True if modifications are complete and ready to execute for verification. False otherwise,
    "reason" : explanatory text for the response (insert here if needed)
}}

# In "read_data" mode
{{
    "mode" : "read_data",
    "target_files" : [path/to/file1, path/to/file2, ..., path/to/fileN], 
    "file_slices" : (if necessary, otherwise None) [
        {{
            "file_path" : (file path),
            "start_line" : (start_line of the scope),
            "end_line" : (end_line of the scope),
        }},...
    ]
    "ongoing_in_mode" : true if the "answer" response in "read_data" mode is long and will continue in subsequent responses. false otherwise,
    "ongoing" : true if the response will continue in a different mode. false otherwise,
    "ready_to_execute" : True if modifications are complete and ready to execute for verification. False otherwise,
    "reason" : explanatory text for the response (insert here if needed)
}}
"""

def produce_final_binary(mix_io_dir, build_path, rust_build_path, run_test_path, run_all_path, run_all_template_path, rust_io_dir, c_io_dir, 
                            raw_dir, meta_dir, work_dir, target_dir, rust_output_dir, database_dir, chat_dir, log_dir, token_path, execute_path,
                            dep_json_path, c_rust_path, rust_c_path, time_path, given_time, target, explore_time, notes,
                            llm_interface, progress_queue, max_iterations, target_path
                            ):
    
    lib_path = f"{rust_io_dir}/src/lib.rs" 
    build_rs_path = rust_io_dir + '/build.rs'
    toml_path = f"{rust_io_dir}/Cargo.toml"
    rust_binary_path = f"{rust_io_dir}/target/release/{target}"

    convert_cargo_toml_to_binary(toml_path, target)

    target_functions = []
    with open(target_path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            parts = line.split(':')
            if len(parts) >= 3:
                target_functions.append({
                    'name': parts[0],
                    'file_path': parts[1],
                    'start_line': int(parts[2]),
                })
                
    if len(target_functions) == 1:
        main_path = f"{rust_io_dir}/src/main.rs"
        if os.path.exists(lib_path):
            os.rename(lib_path, main_path)
            
        prompt = [f"Now we would like to convert Rust code from an FFI wrapper pattern to a standalone binary. Please apply the following modifications to the Rust code below. Follow these rules and steps STRICTLY:",
                    "",
                    "## Strict rules",
                    f"Step A: Modification to {main_path}",
                    "  1. Rename the function `rust_main` to `main`.",
                    "  2. Replace the argument signature of `main`:",
                    "     - If `rust_main` takes `Vec<String>` or `args: Vec<String>`, keep that parameter but populate it inside `main` via `let args: Vec<String> = std::env::args().collect();`.",
                    "     - `main` function signature MUST be exactly: `fn main()` (no parameters, no return type, or return `()`).",
                    "     - If the original `rust_main` returned `i32`, exit with that code using `std::process::exit(code)` at the end.",
                    "  3. Remove the entire `parse_args` function (or any helper that parses `argc`/`argv` from raw pointers).",
                    "  4. Remove the entire `rust_main_wrapper` function (the `extern \"C\"` entry point).",
                    "  5. Remove any `#[no_mangle]`, `#[unsafe(no_mangle)]`, `extern \"C\"`, or `pub extern \"C\"` attributes/modifiers in the code being transformed.",
                    "  6. Keep ALL other functions, types, constants, modules, and imports exactly as they are.",
                    "  7. Keep the body of `rust_main` (now `main`) EXACTLY the same, except for the argument population line added at the top.",
                    "",
                    # f"Step B: Modification to {toml_path}",
                    # "1. If there is a `[lib]` section, remove it entirely.",
                    # "2. Add a `[[bin]]` section with:",
                    # f"   - `name = \"{target}\"`",
                    # "   - `path = \"src/main.rs\"`",
                    # "3. If `crate-type` is specified anywhere, remove it.",
                    # "4. Keep ALL other sections (`[package]`, `[dependencies]`, `[features]`, `[profile.*]`, etc.) exactly as they are.",
                    # "5. Keep all dependency names and version specifications unchanged.",
                    # "",
                    f"Step B: Modification to {run_test_path}",
                    f" 1. Replace invocations of the C binary (which internally calls the Rust FFI wrapper) with direct invocations of the Rust binary.",
                    f"    - The Rust binary is located at: `{rust_binary_path}`",
                    "  2. Keep all test cases, expected outputs, and comparison logic exactly the same.",
                    "  3. Keep the structure of the script (function definitions, loops, variable assignments) identical.",
                    "  4. If the C binary was invoked with arguments, pass the same arguments to the Rust binary.",
                    "  5. Keep all shebang lines, environment variable setup, and cleanup logic unchanged.",
                    ]

    else:
        identifiers = [sanitize_identifier(func['file_path']) for func in target_functions]
        bin_dir = f"{rust_io_dir}/src/bin"

        prompt = [f"Now we would like to convert Rust code from an FFI wrapper pattern to multiple standalone binaries (one binary per sample). The original {lib_path} contains one rust_main_<identifier> per sample, and each must become its own binary file under {bin_dir}/. Follow these rules and steps STRICTLY:",
                    "",
                    "## Strict rules",
                    f"Step A: Modification to {lib_path} and {bin_dir}/",
                    f"  1. For each rust_main_<identifier> currently defined in {lib_path}, create {bin_dir}/<identifier>.rs containing exactly:",
                    "         use trans_rust::*;",
                    "         fn main() {",
                    "             let args: Vec<String> = std::env::args().collect();",
                    "             let code: i32 = { /* body of rust_main_<identifier> here, copied verbatim */ };",
                    "             std::process::exit(code);",
                    "         }",
                    "     The function header `fn rust_main_<identifier>(args: Vec<String>) -> i32 {` is replaced by `fn main() { ... let code: i32 = {`, and the closing `}` of the original function becomes `}; std::process::exit(code); }`. The body itself is copied UNCHANGED.",
                    f"  2. From {lib_path}, REMOVE the following items:",
                    "       - All rust_main_<identifier> functions (now moved to bin files)",
                    "       - The parse_args function",
                    "       - All rust_main_wrapper_<identifier> functions",
                    "       - Any `#[no_mangle]`, `#[unsafe(no_mangle)]`, `extern \"C\"`, or `pub extern \"C\"` attributes/modifiers",
                    f"  3. Keep ALL other items in {lib_path} (types, helper functions, constants, imports, modules) exactly as they are. Add `pub` to any item that becomes referenced from a bin file but was not previously public.",
                    "  4. The bin files must NOT contain copies of shared types or helpers — those stay in lib.rs and are imported via `use trans_rust::*;`.",
                    "",
                    f"Step B: Modification to {run_test_path}",
                    "  1. Replace each invocation of a C binary with the corresponding Rust binary. The mapping is:",
                ]

        for func in target_functions:
            identifier = sanitize_identifier(func['file_path'])
            rust_bin = f"{rust_io_dir}/target/release/{identifier}"
            prompt.append(f"       - C binary derived from {func['file_path']} → {rust_bin}")

        prompt.extend([
                    "  2. Keep all test cases, expected outputs, and comparison logic exactly the same.",
                    "  3. Keep the structure of the script (function definitions, loops, variable assignments) identical.",
                    "  4. If a C binary was invoked with arguments, pass the same arguments to the Rust binary.",
                    "  5. Keep all shebang lines, environment variable setup, and cleanup logic unchanged.",
                ])


    prompt.extend(["\n## Response format", "Please write the answer in the following JSON format.",])
    prompt.extend([reformat_response])

    if len(target_functions) == 1:
        code = read_file(main_path)
        prompt.extend([f"## Rust code ({main_path}):",])
        prompt.extend([code])

    code = read_file(run_test_path)

    prompt.extend([f"## Test code ({run_test_path}):",])
    prompt.extend([code])

    prompt.extend(["", "## Directory structure of the translated Rust program:"]) 
    directory_structure = get_dir_struct("translation", mix_io_dir, None)
    
    write_file(f"{database_dir}/directry_structure.txt", directory_structure)
    directory_structure = trim_code(f"{database_dir}/directry_structure.txt", directory_structure, 10000)
    prompt.extend([directory_structure, ""])

    # ongoing_flag = None
    # error = None
    # std_out = None 
    iteration_count = 0
    progress_queue = None
    max_iterations = 10

    max_retries = 10

    for retry in range(max_retries):
        ongoing_flag = None
        ongoing_in_mode_flag = None
        ready_to_execute = False    
        error = None
        std_out = None 
        read_prompt = None
        mode = None

        sum_target_list = []
        sum_slice_list = []
        sum_modified_list = []
        
        while (1):
            if ongoing_flag is False and ongoing_in_mode_flag is False:
                break
                
            if read_prompt is not None:
                prompt.extend(["", "## Response to the previous request:"])
                prompt.extend(read_prompt)
                read_prompt = None # initialization

                sum_target_list = []
                sum_slice_list = []
                
            rsp_json = ask_llm(prompt, "continue", llm_interface)

            if 'mode' in rsp_json:
                mode = rsp_json['mode']

                if mode == 'read_data':
                    if 'answer' in rsp_json:
                        code = rsp_json['answer']
                        append_file(execute_path, code)

                    if 'target_files' in rsp_json:
                        target_list = rsp_json['target_files']
                        if not isinstance(target_list, list):
                            target_list = [target_list]
                        sum_target_list.extend(target_list)
                    
                    if 'file_slices' in rsp_json and rsp_json['file_slices'] is not None:
                        slice_list = rsp_json['file_slices']
                        if not isinstance(slice_list, list):
                            slice_list = [slice_list]
                        sum_slice_list.extend(slice_list)
                
                if mode == 'modify_data':
                    if 'answer' in rsp_json:
                        modified_list = rsp_json['answer'] # It might be okay to insert individually converted results here
                        if not isinstance(modified_list, list):
                            modified_list = [modified_list]
                        sum_modified_list.extend(modified_list)
            
            ongoing_flag = False
            if 'ongoing' in rsp_json:
                ongoing_flag = rsp_json['ongoing']
            
            ongoing_in_mode_flag = False
            if 'ongoing_in_mode' in rsp_json:
                ongoing_in_mode_flag = rsp_json['ongoing_in_mode']
            
            ready_to_execute = False
            if 'ready_to_execute' in rsp_json:
                ready_to_execute = rsp_json['ready_to_execute']

            if ready_to_execute is True:
                break

            if mode == 'read_data':
                print(f"In mode: {mode}")
                read_prompt = ["- The content obtained in read_data mode is as follows.", ""] 

                slice_set = set()
                for slice_item in sum_slice_list:
                    slice_set.add(slice_item['file_path'])

                new_sum_target_list = []
                print(f"sum_target_list: {sum_target_list}")
                for see_path in sum_target_list:
                    if see_path not in slice_set:
                        new_sum_target_list.append(see_path)
                sum_target_list = new_sum_target_list

                sum_target_list = list(set(sum_target_list))

                tmp_sum_target_list = []
                for see_path in sum_target_list:
                    if see_path is None:
                        continue
                    if not os.path.exists(see_path):
                        see_path = find_matching_path(raw_dir, see_path)
                    tmp_sum_target_list.append(see_path)
                sum_target_list = tmp_sum_target_list

                for see_path in sum_target_list:
                    is_excluded = check_excluded(target_dir, see_path)
                    if is_excluded:
                        continue

                    file_code = get_lined_code(see_path, database_dir)
                    file_code = trim_code(see_path, file_code, 10000)

                    read_prompt.extend([f"- Content of the file {see_path}:"])
                    if not is_empty_string(file_code):
                        read_prompt.extend([f'{file_code}\n'])
                    else:
                        read_prompt.extend([f'None\n'])
                
                for see_item in sum_slice_list:
                    is_excluded = check_excluded(raw_dir, see_item['file_path'])
                    if is_excluded:
                        continue

                    file_code = get_lined_specific_code(database_dir, see_item['file_path'], see_item['start_line'], see_item['end_line'], raw_dir)
                    file_code = trim_code(see_item['file_path'], file_code, 10000)

                    read_prompt.extend([f"Content of {see_item['start_line']} - {see_item['end_line']} lines in the file {see_item['file_path']}:"])
                    read_prompt.extend([f'{file_code}\n'])

            if len(target_functions) == 1:
                prompt = []
                prompt.extend([
                    "Please continue your response.",
                    "Please follow the rules below when modifying the program.",
                    "\n## Modification rules:",
                    "### Step A: Modification to src/main.rs",
                    "1. Rename the function `rust_main` to `main`.",
                    "2. If `rust_main` takes `args: Vec<String>`, populate it inside `main` via `let args: Vec<String> = std::env::args().collect();`.",
                    "3. The `main` function signature MUST be exactly: `fn main()` (no parameters).",
                    "4. If the original `rust_main` returned `i32`, exit with that code using `std::process::exit(code)` at the end.",
                    "5. Remove the entire `parse_args` function.",
                    "6. Remove the entire `rust_main_wrapper` function.",
                    "7. Remove any `#[no_mangle]`, `#[unsafe(no_mangle)]`, `extern \"C\"`, or `pub extern \"C\"` attributes.",
                    "8. Keep ALL other functions, types, constants, modules, and imports exactly as they are.",
                    "9. Keep the body of `rust_main` (now `main`) EXACTLY the same, except for the argument population line added at the top.",
                    "",
                    # "### Step B: Modification to Cargo.toml",
                    # "1. If there is a `[lib]` section, remove it entirely.",
                    # f"2. Add a `[[bin]]` section with `name = \"{target}\"` and `path = \"src/main.rs\"`.",
                    # "3. If `crate-type` is specified anywhere, remove it.",
                    # "4. Keep ALL other sections (`[package]`, `[dependencies]`, etc.) exactly as they are.",
                    # "5. Keep all dependency names and version specifications unchanged.",
                    # "",
                    "### Step B: Modification to run_test.sh",
                    #f"1. Replace invocations of the C binary with direct invocations of the Rust binary at `./target/release/{target}`.",
                    f"1. Replace invocations of the C binary with direct invocations of the Rust binary at `{rust_binary_path}`.",
                    "2. Keep all test cases, expected outputs, and comparison logic exactly the same.",
                    "3. Keep the structure of the script (function definitions, loops, variable assignments) identical.",
                    "4. Pass the same arguments to the Rust binary that were passed to the C binary.",
                    "5. Keep all shebang lines, environment variable setup, and cleanup logic unchanged.",
                ])

            else:
                prompt = []
                prompt.extend([
                    "Please continue your response.",
                    "Please follow the rules below when modifying the program.",
                    "\n## Modification rules:",
                    f"### Step A: Modification to {lib_path} and {rust_io_dir}/src/bin/",
                    f"1. For each rust_main_<identifier> currently defined in {lib_path}, create {rust_io_dir}/src/bin/<identifier>.rs containing exactly:",
                    "       use trans_rust::*;",
                    "       fn main() {",
                    "           let args: Vec<String> = std::env::args().collect();",
                    "           let code: i32 = { /* body of rust_main_<identifier>, copied verbatim */ };",
                    "           std::process::exit(code);",
                    "       }",
                    f"2. From {lib_path}, REMOVE all rust_main_<identifier> functions, the parse_args function, all rust_main_wrapper_<identifier> functions, and any `#[no_mangle]`, `#[unsafe(no_mangle)]`, `extern \"C\"`, or `pub extern \"C\"` attributes.",
                    f"3. Keep ALL other items in {lib_path} (types, helpers, constants, imports, modules) exactly as they are. Add `pub` only to items that become referenced from a bin file.",
                    "4. The bin files must NOT duplicate shared types or helpers — those stay in lib.rs and are imported via `use trans_rust::*;`.",
                    "5. Keep each function body EXACTLY the same as the original rust_main_<identifier>, except for the argument population line at the top.",
                    "",
                    f"### Step B: Modification to {run_test_path}",
                    "1. Replace each invocation of a C binary with the corresponding Rust binary. The mapping is:",
                ])

                for func in target_functions:
                    prompt.extend([f"   - C binary derived from {func['file_path']} → {rust_io_dir}/target/release/{sanitize_identifier(func['file_path'])}"])

                prompt.extend([
                    "2. Keep all test cases, expected outputs, and comparison logic exactly the same.",
                    "3. Keep the structure of the script (function definitions, loops, variable assignments) identical.",
                    "4. Pass the same arguments to each Rust binary that were passed to its corresponding C binary.",
                    "5. Keep all shebang lines, environment variable setup, and cleanup logic unchanged.",
                ])

                
            prompt.extend(["\n## Response format", "Please write the answer in the following JSON format.",
            ])
            prompt.extend([reformat_response])

        if mode == 'modify_data':
            print(f"In mode: {mode}")
            reflect_line_modification(sum_modified_list, raw_dir, database_dir)
            sum_modified_list = []
                

        if ready_to_execute is True:
            break

    print("*********** End of produce_final_binary ***********")
      


def rename_one_path(file_path, raw_dir, c_io_dir):
    file_path = remove_base_path(file_path, f"{raw_dir}")
    file_path = f"{c_io_dir}/{file_path}"
    print(f"file_path at rename_one_path(): {file_path}")

    return file_path


def rename_paths(c_run_path, run_test_path, run_all_path, created_paths, target_funcs, raw_dir, c_io_dir):
    c_run_path = rename_one_path(c_run_path, raw_dir, c_io_dir)
    run_test_path = rename_one_path(run_test_path, raw_dir, c_io_dir)
    run_all_path = rename_one_path(run_all_path, raw_dir, c_io_dir)

    new_files = []
    for file_path in created_paths:
        new_path = rename_one_path(file_path, raw_dir, c_io_dir)
        new_files.append(new_path)

    target_new_files = []
    for item in target_funcs:
        file_path = item['def_file_path']
        item['def_file_path'] = rename_one_path(file_path, raw_dir, c_io_dir)
        target_new_files.append(item)


    return c_run_path, run_test_path, run_all_path, new_files, target_new_files 


def initialize(mix_io_dir, chat_dir, logging_path, database_dir, token_path):
    
    delete_directory(chat_dir) 
    create_directory(chat_dir) 

    data = read_json(logging_path)
    data["prompt_id"] = str(0).zfill(4)
    write_json(logging_path, data)
    
    delete_directory(f"{mix_io_dir}/flows")
    delete_directory(f"{mix_io_dir}/analysis")

    delete_directory(f"{mix_io_dir}/flow_results") 

    delete_file(f"{database_dir}/repair_count.txt")
    delete_file(f"{database_dir}/inner_repair.json")
    delete_file(token_path)


keyboard_interrupt_occurred = False
def signal_handler(signum, frame):
    global keyboard_interrupt_occurred
    keyboard_interrupt_occurred = True
    raise KeyboardInterrupt


def get_report():
    print(f"iteration_counts")
    for rust_path in iteration_dict:
        print(f"{rust_path}: {iteration_dict[rust_path]}")
    
    end_time = time.time()
    exec_time = end_time - start_time
    print(f"Execution Time: {exec_time} seconds")

    print("----------- Translation finished -----------")

    #if not keyboard_interrupt_occurred:    
    save_report_data(archive_dir, result_path, dep_json_path, meta_dir, target, exec_time)


def set_s_repair_dir(compile_dir, target, work_dir):
    print("Set s_repair...")

    compile_dir = compile_dir.replace(f"/{target}", "")

    # Delete work_dir if it already exists
    if os.path.exists(work_dir):
        shutil.rmtree(work_dir)
    
    # Copy the contents of compile_dir directly to work_dir
    shutil.copytree(compile_dir, work_dir)
    
    print(f"Copied: {compile_dir} → {work_dir}")
    
    return work_dir


def to_previous(p, user_id):
    # "database_{user_id}" -> "previous_database_{user_id}"
    parts = list(p.parts)
    try:
        idx = parts.index(f"database_{user_id}")
    except ValueError:
        print(f"Error: 'database_{user_id}' not found in path: {p}")
        sys.exit(1)
    parts[idx] = f"previous_database_{user_id}"
    return Path(*parts)


def allrust_semantics_main(config):

    ################################
    #### Configuraion
    ################################

    process_type = config["process_type"]
    user_id = config["user_id"]
    compile_dir = config["compile_dir"]
    llm_choice = config["llm_choice"]
    claude_api_key = config["claude_api_key"]
    azure_endpoint = config["azure_endpoint"]


    occupy_path = None
    given_time = 60

    run_all_template_path = f"{TRANS_HOME}/template/run_all_s_repair.sh"

    # Create PathConfig
    paths = create_path_config(
        user_id=user_id,
        original_dir=compile_dir,
        process_type=process_type,
        work_dir=None,
    )

    (target,
    build_path, 
    rust_build_path,
    rust_lib_h_path,
    run_test_path,
    run_all_path,
    raw_dir,
    target_dir, 
    work_dir, 
    c_code_dir,
    rust_output_dir, 
    execute_path,
    ecute_path,

    meta_dir, 
    div_meta_dir,
    chat_dir, 
    chat_macro_dir, 
    log_dir, 
    exp_dir,
    archive_dir, 

    macro_finder, 
    database_dir,  

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

    chat_dir,
    history_path,
    token_path,
    count_path, 
    time_path,
    output_dir,

    independent_const_build_path, 
    flag_build_path) = extract_all_paths(paths)

    mix_io_dir = work_dir
    c_io_dir = f"{work_dir}/{target}"
    rust_io_dir = f"{work_dir}/trans_rust"

    ################################
    #### Semantics repiar
    ################################

    created_paths = []  # Temporary workaround

    if process_type == "s_repair":

        init_prompt_count(count_path)

        llm_interface = LLMInterface(
            project_id=target,
            occupy_path=occupy_path,
            llm_choice=llm_choice,
            full_regions=full_regions,
            llm_model=None,
            output_max=128000, # 4000,
            context_window=1000000,
            temperature=0,
            api_key=None,
            timeout=300,
            history_path=history_path,
            token_path=token_path,
            database_dir=database_dir,
            chat_dir=chat_dir,
            count_path=count_path,
            exp_data={},
        )

        if TEST_MODE:
            llm_interface = occupy_llm(llm_interface)
        else:
            llm_model = get_claude_model(llm_choice)
            llm_interface = configure_llm(
                llm_interface,
                claude_api_key,
                azure_endpoint,
                llm_model
            )

        if TEST_MODE:
            atexit.register(lambda: shutdown_llm(llm_interface))

        progress_queue = []
        max_iterations = 5
        explore_time = 0
        notes = []

        """
        # Repair function errors
        check_semantics(mix_io_dir, build_path, rust_build_path, run_test_path, run_all_path, run_all_template_path, rust_io_dir, c_io_dir, 
                        raw_dir, meta_dir, work_dir, target_dir, rust_output_dir, database_dir, chat_dir, log_dir, token_path, execute_path,
                        dep_json_path, c_rust_path, rust_c_path, time_path, given_time, target, explore_time, notes,
                        llm_interface, progress_queue, max_iterations, False
                        )  
        """
        set_s_repair_dir(compile_dir, target, work_dir)

        signal.signal(signal.SIGINT, signal_handler)
        start_time = time.time()
        log_file_path = set_log(log_dir, llm_choice, target, logging_path, 's_repair', DEBUG_LLM)
        
        # atexit.register(get_report)
        setup_rust_trace(work_dir)

        # Initialize
        initialize(mix_io_dir, chat_dir, logging_path, database_dir, token_path) 

        # Repair function errors
        check_semantics(mix_io_dir, build_path, rust_build_path, run_test_path, run_all_path, run_all_template_path, rust_io_dir, c_io_dir, 
                        raw_dir, meta_dir, work_dir, target_dir, rust_output_dir, database_dir, chat_dir, log_dir, token_path, execute_path,
                        dep_json_path, c_rust_path, rust_c_path, time_path, given_time, target, explore_time, notes,
                        llm_interface, progress_queue, max_iterations, True
                        )  

        # Delete previous_workspace_s_repair_{user_id}
        if os.path.exists(f"previous_workspace_s_repair_{user_id}/{target}"):
            delete_directory(f"previous_workspace_s_repair_{user_id}/{target}")

        copy_directory(work_dir, f"previous_workspace_s_repair_{user_id}/{target}")

        if FFI_STRATEGY == "minimize":
            target_path = f"{target_dir}/actual_targets.txt"

            # Produce the final standalone binary
            produce_final_binary(mix_io_dir, build_path, rust_build_path, run_test_path, run_all_path, run_all_template_path, rust_io_dir, c_io_dir, 
                            raw_dir, meta_dir, work_dir, target_dir, rust_output_dir, database_dir, chat_dir, log_dir, token_path, execute_path,
                            dep_json_path, c_rust_path, rust_c_path, time_path, given_time, target, explore_time, notes,
                            llm_interface, progress_queue, max_iterations, target_path
                            )  

            # Repair function errors
            check_semantics(mix_io_dir, build_path, rust_build_path, run_test_path, run_all_path, run_all_template_path, rust_io_dir, c_io_dir, 
                            raw_dir, meta_dir, work_dir, target_dir, rust_output_dir, database_dir, chat_dir, log_dir, token_path, execute_path,
                            dep_json_path, c_rust_path, rust_c_path, time_path, given_time, target, explore_time, notes,
                            llm_interface, progress_queue, max_iterations, False
                            )  

        output = {
            'work_dir' : work_dir
        }

        if os.path.exists(token_path):
            cost = calc_claude_cost_from_file(token_path)
            print(f"Total cost: ${cost['total_cost_usd']:.2f}")

        print("\n\n++++++++++++++= End of s_repair process ++++++++++++++=\n")
    

    elif process_type == "stash":  

        metadata_dir = config["meta_dir"]
        div_metadata_dir = config["div_meta_dir"]
        block_output_path = config["block_path"]

        metadata_path = Path(metadata_dir)
        div_metadata_path = Path(div_metadata_dir)
        block_output_file = Path(block_output_path)

        # Compute previous_* destinations by inserting "previous_" before the directory name.
        # e.g. /root/.../trans/metadata_{user_id}/avl -> /root/.../trans/previous_metadata_{user_id}/avl
        previous_metadata_path = metadata_path.parent.parent / f"previous_{metadata_path.parent.name}" / metadata_path.name
        previous_div_metadata_path = div_metadata_path.parent.parent / f"previous_{div_metadata_path.parent.name}" / div_metadata_path.name
        # previous_block_output_file = block_output_file.parent / f"previous_{block_output_file.name}"
        previous_block_output_file = to_previous(block_output_file, user_id)

        # Validate that the sources exist before touching anything.
        for src in [metadata_path, div_metadata_path, block_output_file]:
            if not src.exists():
                print(f"Error: source does not exist: {src}")
                sys.exit(1)

        # Stash metadata directory
        print(f"Stashing {metadata_path} -> {previous_metadata_path}")
        if previous_metadata_path.exists():
            shutil.rmtree(previous_metadata_path)
        previous_metadata_path.parent.mkdir(parents=True, exist_ok=True)
        shutil.copytree(metadata_path, previous_metadata_path)

        # Stash div_metadata directory
        print(f"Stashing {div_metadata_path} -> {previous_div_metadata_path}")
        if previous_div_metadata_path.exists():
            shutil.rmtree(previous_div_metadata_path)
        previous_div_metadata_path.parent.mkdir(parents=True, exist_ok=True)
        shutil.copytree(div_metadata_path, previous_div_metadata_path)

        # Stash block_output.txt (copy, not move — keep the original safe)
        print(f"Stashing {block_output_file} -> {previous_block_output_file}")
        previous_block_output_file.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(block_output_file, previous_block_output_file)

        print("\n\n++++++++++++++= End of stash process ++++++++++++++=\n")
    

if __name__ == "__main__":
    
    #####################################################################
    ##### Input
    #####################################################################

    # setup
    process_type = str(sys.argv[1])

    if process_type not in ["s_repair", "stash"]:
        raise ValueError(f"Error: invalid process_type '{process_type}'. Expected one of: s_repair, stash")

    compile_dir = None
    meta_dir = None
    div_meta_dir = None
    block_path = None
    
    if process_type == "s_repair":
        compile_dir = str(sys.argv[2])
    
    elif process_type == "stash":
        meta_dir = str(sys.argv[2])
        div_meta_dir = str(sys.argv[3])
        block_path = str(sys.argv[4])

    rust_edition = "2024"
    user_id = "0000"

    config_path = f"{CONFIG_PATH}"
    config_data = read_json(config_path)

    llm_choice = config_data["llm_choice"]
    claude_api_key = config_data["claude_api_key"]
    azure_endpoint = config_data["azure_endpoint"]
    TEST_MODE = config_data["test_mode"] 
    FFI_STRATEGY = config_data["ffi_strategy"] 
    

    config = {
        "process_type": process_type,
        "compile_dir": compile_dir,
        "block_path" : block_path,
        "meta_dir": meta_dir,
        "div_meta_dir": div_meta_dir,
        "user_id": user_id,
        "rust_edition" : rust_edition,
        "llm_choice": llm_choice,
        "claude_api_key": claude_api_key,
        "azure_endpoint": azure_endpoint,
    }

    allrust_semantics_main(config)


