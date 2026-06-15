import os
import json
import networkx as nx
import matplotlib.pyplot as plt
from typing import Dict, List, Tuple, Set
import subprocess
import re
import copy
from typing import List            
from copy import deepcopy
from collections import defaultdict, deque
import shutil
import sys
import clang.cindex
from clang.cindex import CursorKind
import textwrap
import toml
import logging
import base64
import atexit
import signal
import random
import math
import time
from pathlib import Path
import tempfile
from typing import Dict, Optional
import platform
import datetime
from typing import Set, List
from typing import List, Dict
import tiktoken

from utils_api import (
    # normal
    read_json,
    write_json,
    read_file,
    write_file,
    copy_file,
    delete_file,
    create_file,
    recreate_file,
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
    merge_json,
    merge_list,
    delete_lines,
    load_toml_file,
    write_toml_file,
    write_toml,
    json_to_toml,
    merge_toml_json,
    get_lined_specific_code,

    # translation specific
    set_log,
    create_path_config,
    extract_all_paths,
    obtain_metadata,
    execute_command,
    find_highest_source,
    remove_base_path,
    update_path_map,
    calculate_execution_time,
    get_lined_code,
    get_unit_code,
    normalize_translation_metadata,
    normalize_translation_div_metadata,
    denormalize_translation_metadata,
    denormalize_translation_div_metadata,
    denormalize_dep_data,
    denormalize_target_path,
    denormalize_block_group_path,
    denormalize_block_path,
    normalize_metafiles,
    denormalize_metafiles,
    update_parent_path,
    get_ref_files,
    get_path_map,
    read_specific_lines,
    append_rust_path,
    update_modified_keys,
    get_name_key,
    parse_def_loc,
)

from llm_api import (
    TransConfig,
    CorConfig,
    LLMInterface,
    init_prompt_count, 
    occupy_llm,
    configure_llm,
    shutdown_llm,
    save_coverage_report,
    get_dir_struct,
    ask_llm,
    reflect_line_modification,
    get_modified_rust_lines,
    get_grouped_c_keys,
    merge_with_initial,
    get_claude_model,
    find_matching_path,
    adjust_prompt,
    ask_correspondence,
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
    find_headers,
    get_build_path,
    get_entry_points,
    generate_is_program,
    get_compile_json,
    clone_compile_json,
    is_system_file,
    find_binaries,
)

from rust_parser_api import (
    parse_files_rust,
    get_rust_interface,
    rust_find_function_end,
    update_c_rust_metadata,
    merge_c_rust_metadata,
    update_metadata_with_rust,
    setup_rust_trace,
)

MACRO_HOME = "/root/SmartC2Rust/macro"
TRANS_HOME = "/root/SmartC2Rust/trans"
C_PARSER_HOME = "/root/kiso-parser-c"
CONFIG_PATH = "/root/SmartC2Rust/config.json"


####################################################
###  Configurations
####################################################

FFI_STRATEGY = None
W_O_DEP = False # default False    # Without dependency data

MANUAL_FIRST = False
MANUAL = False
DEBUG_LLM = False

WITH_CONDENSED = WITH_FILES = False

INSERT_FILES = False
INSERT_FILES_REPAIR = False

REPAIR_MEMORIZED = True

FROM_PART = False #True
REPAIR_MAX = 500
LLM_DIV = False
RULE_DIV = True

MOD_LINE = True #False # Modify line by line
MOD_PARSE = not(MOD_LINE) # Modify per parse unit

####################################################
# Initial values
####################################################
TEST_MODE = None 

keyboard_interrupt_occurred = False
path_count = 0  
current_c_block_end = 1 #0

finished = False
platform_instruction = ""
reflect_path = None

reflect_count = 0

SINGLE_TYPE = DIVIDED_TYPE = PLAIN_TYPE = False

iteration_dict = {}
judge_dict = {}
persable_units = {}
module_list = []
full_regions = []


#########################################################
# Setup for translated Rust directory
#########################################################

def remove_git_directory(rust_directory):
    """
    Remove the .git directory inside the specified Rust project directory
    
    Parameters:
    -----------
    rust_directory : str
        Directory path of the Rust project
    """
    git_path = os.path.join(rust_directory, '.git')
    
    try:
        if os.path.exists(git_path):
            # Recursively remove the .git directory
            shutil.rmtree(git_path)
            print(f"Removed .git directory from {rust_directory}")
        else:
            print(f"No .git directory found in {rust_directory}")
    except Exception as e:
        print(f"Error removing .git directory: {e}")


def add_tracing(rust_io_dir, toml_path):    
    # Read Cargo.toml
    with open(toml_path, 'r') as f:
        content = f.read()
    
    # Add the [lib] section
    if '[lib]' not in content:
        # Add the [lib] section if it does not exist
        #lib_section = '\n[lib]\nname = "trans_rust"\ncrate-type = ["cdylib"]\n'
        lib_section = '\n[lib]\nname = "trans_rust"\ncrate-type = ["staticlib"]\n'
        # Add the [lib] section after the package section
        if '[package]' in content:
            parts = content.split('[package]')
            package_parts = parts[1].split('[', 1)
            content = '[package]' + package_parts[0] + lib_section + ('[' + package_parts[1] if len(package_parts) > 1 else '')

    # Find the [dependencies] section
    if '[dependencies]' not in content:
        # Add the [dependencies] section if it does not exist
        content += '\n[dependencies]\n'

    dependency_line = f'''tracing = "0.1"
tracing-subscriber = {{ version = "0.3", features = ["env-filter", "json"] }}'''

    # Check whether the dependencies have already been added
    #if f'{lib_name}' not in content:
        # Add after the [dependencies] section
    parts = content.split('[dependencies]')
    content = parts[0] + '[dependencies]\n' + dependency_line + (parts[1] if len(parts) > 1 else '')

    # Write the changes back
    with open(toml_path, 'w') as f:
        f.write(content)


def create_rust_libdir(work_dir, rust_output_dir):
    #delete_directory(rust_output_dir)
    commands = [f"cargo new {rust_output_dir} --lib"]
    output, error = execute_command(commands)
    if output:
        print("Output:", output)
    if error:
        print("Error:", error)
    
    # (re-)create rust directory # insert build.rs
    lib_path = f"{rust_output_dir}/src/lib.rs" 
    build_rs_path = rust_output_dir + '/build.rs'
    toml_path = f"{rust_output_dir}/Cargo.toml"

    #delete_file(lib_path)
    recreate_file(lib_path)

    # Remove git files  # added
    remove_git_directory(rust_output_dir)

    # Create each file
    base_path = find_highest_source(work_dir)
    lowest_dir = os.path.basename(os.path.normpath(base_path))

    if (MANUAL_FIRST or not MANUAL) or not DEBUG_LLM:
        print("Creating each module file")
        
        with open(lib_path, "w") as lib_file:
            lib_file.write(f"#![allow(non_upper_case_globals)]\n")
            lib_file.write(f"#![allow(non_camel_case_types)]\n")
            lib_file.write(f"#![allow(non_snake_case)]\n")
            lib_file.write(f"\n")
            lib_file.write(f"include!(concat!(env!(\"OUT_DIR\"), \"/bindings.rs\"));\n")  # Write include! in the file that wants to use the contents of bindings.rs

    add_tracing(rust_output_dir, toml_path)

    return build_rs_path, lib_path, toml_path 


def create_rust_libdir(work_dir, rust_output_dir):

    commands = [f"cargo new {rust_output_dir} --lib"]
    output, error = execute_command(commands)
    if output:
        print("Output:", output)
    if error:
        print("Error:", error)
    
    # (re-)create rust directory # insert build.rs
    lib_path = f"{rust_output_dir}/src/lib.rs"
    build_rs_path = rust_output_dir + '/build.rs'
    toml_path = f"{rust_output_dir}/Cargo.toml"

    recreate_file(lib_path)

    # Remove git files
    remove_git_directory(rust_output_dir)

    # Create each file
    base_path = find_highest_source(work_dir)
    lowest_dir = os.path.basename(os.path.normpath(base_path))

    if (MANUAL_FIRST or not MANUAL) or not DEBUG_LLM:
        print("Creating each module file")
        
        #create_file(build_rs_path) #delete_file(lib_path)
        with open(lib_path, "w") as lib_file:
            lib_file.write(f"#![allow(non_upper_case_globals)]\n")
            lib_file.write(f"#![allow(non_camel_case_types)]\n")
            lib_file.write(f"#![allow(non_snake_case)]\n")
            lib_file.write(f"\n")
            lib_file.write(f"include!(concat!(env!(\"OUT_DIR\"), \"/bindings.rs\"));\n")  # Write include! in the file that wants to use the contents of bindings.rs
    
    add_tracing(rust_output_dir, toml_path)

    return build_rs_path, lib_path, toml_path


def create_rust_bindir(work_dir, rust_output_dir):
    commands = [f"cargo new {rust_output_dir} --bin"]
    output, error = execute_command(commands)
    if output:
        print("Output:", output)
    if error:
        print("Error:", error)

    main_path = f"{rust_output_dir}/src/main.rs"
    build_rs_path = f"{rust_output_dir}/build.rs"
    toml_path = f"{rust_output_dir}/Cargo.toml"

    recreate_file(main_path)

    # Remove git files
    remove_git_directory(rust_output_dir)

    base_path = find_highest_source(work_dir)
    lowest_dir = os.path.basename(os.path.normpath(base_path))

    if (MANUAL_FIRST or not MANUAL) or not DEBUG_LLM:
        print("Creating each module file")

        with open(main_path, "w") as main_file:
            main_file.write(f"#![allow(non_upper_case_globals)]\n")
            main_file.write(f"#![allow(non_camel_case_types)]\n")
            main_file.write(f"#![allow(non_snake_case)]\n")
            main_file.write(f"\n")
            main_file.write(f"include!(concat!(env!(\"OUT_DIR\"), \"/bindings.rs\"));\n")
            main_file.write(f"\n")
            main_file.write(f"fn main() {{\n")
            main_file.write(f"    // TODO: entry point\n")
            main_file.write(f"}}\n")

    add_tracing(rust_output_dir, toml_path)

    return build_rs_path, main_path, toml_path



def get_existing_lib_paths(work_dir, rust_output_dir):
    
    # (re-)create rust directory # insert build.rs
    lib_path = f"{rust_output_dir}/src/lib.rs"
    build_rs_path = rust_output_dir + '/build.rs'
    toml_path = f"{rust_output_dir}/Cargo.toml"

    return build_rs_path, lib_path, toml_path


def write_lib(lib_path, targetectory, target):
    # Create the lib.rs file
    base_path = find_highest_source(targetectory)
    lowest_dir = os.path.basename(os.path.normpath(base_path))

    if (MANUAL_FIRST or not MANUAL) or not DEBUG_LLM:

        with open(lib_path, "a") as lib_file:
            lib_file.write(f"\n\n")
            lib_file.write(f"// added a module\n")
            mod_name = change_hyphn(target)
            lib_file.write(f"pub mod {mod_name};\n")


def add_module_declaration(mod_path, module_name):
    """
    Add a module declaration to the mod.rs file
    Do not add it if the declaration already exists
    """
    new_mod_name = change_hyphn(module_name)
    declaration = f"pub mod {new_mod_name};"
    
    # Read existing declarations
    existing_declarations = set()
    if os.path.exists(mod_path):
        with open(mod_path, 'r') as f:
            existing_declarations = set(f.read().splitlines())
    
    # Add only if the declaration does not exist
    if declaration not in existing_declarations:
        with open(mod_path, 'a') as f:
            # Add a newline if the file is not empty
            if existing_declarations:
                f.write('\n')
            f.write(declaration + '\n')


def change_hyphn(name):
    name = name.replace('-', '_')
    name = name.replace('+', 'plus')
    #name = name.replace('.', '_')

    # Check whether it starts with a digit
    if name and name[0].isdigit():
        # Convert the digit to a word
        number_words = {
            '0': 'zero',
            '1': 'one',
            '2': 'two',
            '3': 'three',
            '4': 'four',
            '5': 'five',
            '6': 'six',
            '7': 'seven',
            '8': 'eight',
            '9': 'nine'
        }
        
        # Convert only the first digit to a word
        first_digit = name[0]
        name = number_words[first_digit] + name[1:]

    return name



def search_key(key, rsp_json):
    key_flag = True
    if key not in rsp_json:
        key_flag = False

    else: # 'error_path' in rsp_json:
        if  rsp_json[key] is None:
            key_flag = False
        if rsp_json[key] == "":
            key_flag = False
        if rsp_json[key] is not None and rsp_json['error_path'].isspace():
            key_flag = False
        
    return key_flag


##############################################
##### Translation helper functions
##############################################

translate_template = f"""# In write_rust mode
{{
    "mode": "write_rust",
    "rust_code": "The Rust code in the response. Since it will be executed as-is, absolutely no omissions or placeholders should be included. Write the actual implementation so that it can be copied and pasted.",
    "c_block_start": The starting line number of the block in the original C code that was translated in the \"rust_code\",
    "c_block_end": The ending line number of the block in the original C code that was translated in the \"rust_code\",
    "no_omission": True if the response code contains no omissions and can be executed as-is. Otherwise, False,
    "ongoing": true if the response is split into multiple parts and there is still remaining JSON data, false otherwise,,
    "unsafe_count": The number of unsafe blocks in the answered \"rust_code\",
    "unsafe_used": True if `unsafe` is used at least once in the provided \"rust_code\". Otherwise, False.,
    "reason": An explanation of the response. The explanation must absolutely include a justification regarding the use of `unsafe` (either proof that it is not used or a compelling reason why its use is unavoidable)."
}}

# In read_data mode
{{
    "mode": "read_data",
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
    "reason" : explanatory text for the response (insert here if needed)
}}
"""


# "current_block_complete": True if the response \"rust_code\" fully implements the functionality of the original C code from c_block_start to c_block_end without any mocking or simplifications. Otherwise, False.,
refine_template = f"""# In write_rust mode
{{ 
    "mode": "write_rust",
    "rust_code": "The Rust code in the response. Since it will be executed as-is, absolutely no omissions or placeholders should be included. Write the actual implementation so that it can be copied and pasted.",
    "rust_block_start": The starting line number of the corresponding Rust block in the \"rust_code\",
    "rust_block_end": The ending line number of the corresponding Rust block in the \"rust_code\",
    "no_omission": True if the response code contains no omissions and can be executed as-is. Otherwise, False,
    "ongoing": true if the response is split into multiple parts and there is still remaining JSON data, false otherwise,
    "unsafe_count": The number of unsafe blocks in the answered \"rust_code\",
    "unsafe_used": True if `unsafe` is used at least once in the provided \"rust_code\". Otherwise, False.,
    "refined_completed" : True if the code has been successfully refined to proper Rust style with improved safety and adherence to Rust principles; otherwise, false,
    "reason": An explanation of the response. The explanation must absolutely include a justification regarding the use of `unsafe` (either proof that it is not used or a compelling reason why its use is unavoidable)."
}}

# In read_data mode
{{
    "mode": "read_data",
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
    "reason" : explanatory text for the response (insert here if needed)
}}
"""


def get_target_function(one_unit, target_path):
    print("Getting FFI boundary functions...")

    target_functions = []
    with open(target_path, 'r') as f:
        target_functions = [line.strip() for line in f if line.strip()]

    target_list = []
    for item in one_unit:
        if 'name' in item:
            name_key = f"{item['name']}:{item['file_path']}:{item['start_line']}"
    
        if 'macro_name' in item and 'start_line' in item:
            name_key = f"{item['macro_name']}:{item['file_path']}:{item['start_line']}"
        
        if 'macro_name' in item and 'line' in item:
            name_key = f"{item['macro_name']}:{item['file_path']}:{item['line']}"
        
        if name_key in target_functions:
            target_list.append(name_key)
        
    return target_list


# Present the definition locations that have already been determined
# For C, request actions with respect to definition locations that are expected to be determined later.
# Note that c_path is used not as the actual file itself, but as something for retrieving meta_dir, so there is a potential intuitive mismatch in the fact that div_c_path does not need to physically exist.
def get_context_prompt(conv_type, prompt, one_unit, dep_json_path, is_program_path, 
                       original_dir, meta_dir, div_meta_dir, rust_output_dir, build_path, target, user_id):  # , macro_path, all_macro_path# , build_list_path

    added_prompt = []
    cashed = {}

    components_included = []
    for item in one_unit:
        components_included.append(item)
        if 'components' in item:
            components_included.append(item['components'])

    t_at_least_found = False
    i_at_least_found = False
    d_at_least_found = False
    g_at_least_found = False
    if_at_least_found = False
    r_at_least_found = False

    rust_refs = {}
    independent_macros = {}
    dependent_macros = {}
    ifdefs = set() #{}
    global_vars = {}
    targets = {}
    seen = set()

    g_used = False
    f_used = False
    i_used = False

    data = {}
    for c_item in components_included:
        c_name = c_item['name']
        c_path = c_item['file_path']
        start_line = c_item['start_line']
        end_line = c_item['end_line']

        if c_path not in data:
            data[c_path] = []
        data[c_path].append(c_item)

        """
        # Maybe put a sys_macro flag in here and change added_prompt based on that
        added_prompt, cashed, i_at_least_found, independent_macros, if_at_least_found, ifdefs, r_at_least_found, rust_refs, g_at_least_found, global_vars = collect_dependencies(cashed, c_item, dep_json_path, div_meta_dir, original_dir, build_path, conv_type, i_at_least_found, independent_macros, if_at_least_found, ifdefs, r_at_least_found, rust_refs, g_at_least_found, global_vars)
        sum_prompt.extend(added_prompt)
        """

    dependencies = {}
    for file_path, c_items in data.items():
        meta_path = obtain_metadata(file_path, div_meta_dir, False, True, "def")

        if meta_path not in cashed:
            meta_data = obtain_metadata(file_path, div_meta_dir, False, False, "def")
            cashed[meta_path] = {}
            cashed[meta_path] = meta_data
        else:
            meta_data = cashed[meta_path]   
        
        dependencies = collect_dependencies(cashed, c_items, meta_path, meta_data, dep_json_path, is_program_path,
                                            div_meta_dir, original_dir, build_path, conv_type, 
                                            i_at_least_found, independent_macros, if_at_least_found, ifdefs, r_at_least_found, rust_refs, 
                                            g_at_least_found, global_vars, t_at_least_found, targets, seen, target,
                                            g_used, f_used, i_used, user_id)
        
        # prompt = dependencies["prompt"]
        cashed = dependencies["cashed"]
        t_at_least_found = dependencies["t_at_least_found"]
        targets = dependencies["targets"]

        i_at_least_found = dependencies["i_at_least_found"]
        independent_macros = dependencies["independent_macros"]
        # d_at_least_found = dependencies["d_at_least_found"]
        # dependent_macros = dependencies["dependent_macros"]
        if_at_least_found = dependencies["if_at_least_found"]
        ifdefs = dependencies["ifdefs"]
        r_at_least_found = dependencies["r_at_least_found"]
        rust_refs = dependencies["rust_refs"]
        g_at_least_found = dependencies["g_at_least_found"]
        global_vars = dependencies["global_vars"]

        g_used = dependencies["g_used"]
        f_used = dependencies["f_used"]
        i_used = dependencies["i_used"]

    # targets
    if t_at_least_found:
        if FFI_STRATEGY == "preserve":
            added_prompt.extend([
                "- FFI boundary functions:",
                "    - The following functions already have stub implementations. Please replace them with the actual implementations.",
                "    - Functions:",
            ])
        else:
            added_prompt.extend([
                "- FFI boundary functions:",
                "    - The following functions already have stub implementations. Only replace the body of rust_main_<identifer>(args: Vec<String>) -> i32. Do NOT modify rust_main_wrapper_<identifer>, parse_args, or their signatures. Do NOT create any new function named rust_main_<identifer>.",
                f"    - rust_main_<identifer> MUST call std::io::stdout().flush() before returning, to flush Rust's stdout buffer before control returns to C.",
                "    - Functions:",
                "      - rust_main_<identifer>",
            ])
            
        for target_name, target_info in targets.items():
            added_prompt.append(f"      - {target_name}") 

    # independent constant macros
    if i_at_least_found:
        added_prompt.extend([
            "- Independent constant macros:", 
            f"    - These C macros are translated to Rust constants by bindgen in `bindings.rs`.",
            f"    - They are available via `include!(concat!(env!(\"OUT_DIR\"), \"/bindings.rs\"));`",
            f"    - Use them directly (just write `{{macro_name}}`). Do NOT redefine them.",
            f"    - bindgen blocklists ALL types (blocklist_type(\".*\")) and emits ONLY indenpendent constant macros. It does NOT emit any typedef/struct/enum, nor dependent (referencing) macros.",
            f"    - Note: Some macros may also appear in \"cfg state items\" below. When a macro appears in BOTH lists, use it as a bindgen constant for its VALUE, and use #[cfg(has_<name>)] for CONDITIONAL COMPILATION blocks (#ifdef/#ifndef).",
            f"    - Macros:",
        ])
        for target_name, callers in independent_macros.items():
            added_prompt.extend([f"      - {target_name}"])
            """
            added_prompt.extend([f"      - {target_name} (callee):"])
            added_prompt.append(f"            <- callers: {', '.join(callers)}")
            """
    
    # ifdef statements
    if if_at_least_found:
        ifdef_list = ", ".join(ifdefs)
        added_prompt.extend([
            "- cfg state items:", 
            #"    - Please translate the following #ifdef statements to Rust, taking into consideration that the conditional compilation flags are defined as cfg attributes in build.rs.",
            "    - For EVERY #ifdef/#ifndef statement using a macro listed in \"Macros:\", you MUST wrap the corresponding Rust code with #[cfg(has_<macro_name>)].",
            "    - The conditional compilation flags are automatically detected from C header macros using bindgen, and emitted as cargo:rustc-cfg=has_<macro_name> during the build process.",
            "    - When identical type/function definitions appear under different #ifdef guards, you MUST output each one separately with its own #[cfg(has_<macro_name>)]. In Rust, #[cfg] attributes make them mutually exclusive at compile time, so duplicate names are allowed.",
            "    - Do NOT skip or merge #[cfg] attributes even if the code inside different #ifdef blocks looks identical. Different configurations will be tested separately, and each block must be independently compilable.",
            f"    - Macros: {ifdef_list}"
        ])
    
    # global variables
    if g_at_least_found:
        global_vars_list = ", ".join(global_vars)

        if FFI_STRATEGY == "preserve":
            added_prompt.extend([
                "- Global variables:",
                "    - These C global variables are shared across the FFI boundary.",
                "    - Please follow 2 steps below.",
                "        Step 1: On the Rust side, declare each as: extern \"C\" { static mut <name>: <type>; }",
                "        Step 2: For each variable, provide a getter and setter function following this pattern:",
                "            pub fn rust_get_<name>() -> <type> {",
                "                unsafe { <name> }",
                "            }",
                "            pub fn rust_set_<name>(val: <type>) {",
                "                unsafe { <name> = val; }",
                "            }",
                "    - Note that the C side retains the original definition.",
                "    - Both C and Rust read/write the same memory location through FFI linkage.",
                "    - Variables:",
            ])

        else:
            added_prompt.extend([
                "- Global variables:",
                "    - These C global variables should be translated to safe Rust.",
                "    - Prefer encapsulating related global state into a struct and passing it as a parameter, rather than keeping globals.",
                "    - Use thread-safe types such as static with std::sync::Mutex, std::sync::RwLock, std::sync::OnceLock, or std::sync::atomic types.",
                "    - Do NOT use static mut or unsafe extern \"C\" static declarations.",
                # "    - For example:",
                # "        use std::sync::Mutex;",
                # "        static <NAME>: Mutex<<type>> = Mutex::new(<initial_value>);",
                # "    - Provide getter and setter functions following this pattern:",
                # "        pub fn rust_get_<name>() -> <type> {",
                # "            *<NAME>.lock().unwrap()",
                # "        }",
                # "        pub fn rust_set_<name>(val: <type>) {",
                # "            *<NAME>.lock().unwrap() = val;",
                # "        }",
                "    - Variables:",
            ])


        for var_name, var_info in global_vars.items():
            # If var_info contains type or initial value, present them
            added_prompt.append(f"      - {var_name}") #: {var_info}")


    #############################################################################
    ###### Rust code
    #############################################################################
    
    # rust code
    if r_at_least_found:
        added_prompt.extend([
            "- Rust items being translated:", # Referencing (using) functions
            #"  - The functions being called from each function have already been translated into Rust as shown below."
            "    - Please use the following items that have already been translated into Rust.",
            "    - Rust codes:",
        ])

        for caller_name, rust_item in rust_refs.items():
            added_prompt.extend([f"      - {caller_name} (caller):"])

            callee_name = rust_item['name']
            callee_rust_code = rust_item['rust_code']
            if 'category' in rust_item and rust_item['category'] is not None:
                category = rust_item['category']
                added_prompt.append(f"         - {callee_name} (callee, {category}): {callee_rust_code}") 

            else:
                added_prompt.append(f"         - {callee_name} (callee): {callee_rust_code}") 

    
    #############################################################################
    ###### Access rules
    #############################################################################

    if i_used is True:
        added_prompt.extend([
            "- Independent constant macros access rules:",
            "        - These are already available via `include!(concat!(env!(\"OUT_DIR\"), \"/bindings.rs\"));`",
            "        - Use them directly by name. Do NOT redefine them.",
            f"        - Note: bindgen blocklists ALL types (blocklist_type(\".*\")) and emits ONLY indenpendent constant macros. It does NOT emit any typedef/struct/enum, nor dependent (referencing) macros.",
        ])

    if f_used is True:
        added_prompt.extend([
            "- cfg attributes access rules:",
            "        - These are automatically emitted as `cargo:rustc-cfg=has_<macro_name>` during build.",
            "        - Use them as `#[cfg(has_<name>)]` or `if cfg!(has_<name>)`.",
        ])

    if g_used is True:
        added_prompt.extend([
            "- Global variable access methods:",
            "        - ALWAYS use the getter/setter functions that are already declared.",
            "        - NEVER access extern variables directly.",
            "        - NEVER redeclare them with static, static mut, Mutex, or any other form.",
        ])


    prompt.extend(added_prompt)
    return prompt, added_prompt


# It seems there are no macro-related elements here
def get_rust_context_prompt(conv_type, one_unit, dep_json_path, meta_dir, rust_output_dir, build_path, c_rust_path):  # , macro_path, all_macro_path# , build_list_path
    
    c_rust_map = read_json(c_rust_path)

    sum_conv_prompt = []
    sum_rep_prompt = []

    convert_prompt = []
    repair_prompt = []

    cashed = {}

    for c_item in one_unit:
        c_name = c_item['name']
        c_path = c_item['file_path']
        start_line = c_item['start_line']
        end_line = c_item['end_line']

        convert_prompt, repair_prompt, cashed = collect_rust_dependencies(cashed, c_item, dep_json_path, meta_dir, build_path, conv_type, c_rust_map)
        sum_conv_prompt.extend(convert_prompt)
        sum_rep_prompt.extend(repair_prompt)

    return sum_rep_prompt


def get_encoder(encoding_name="cl100k_base"):
    return tiktoken.get_encoding(encoding_name)


def count_tokens(text, enc=None):
    enc = enc or get_encoder()
    return len(enc.encode(text))


def chunk_by_tokens(code, max_tokens, enc):
    """Split code into chunks of <= max_tokens, line by line.

    Lines are never broken mid-line so the syntax stays intact.
    If a single line exceeds max_tokens, that line is force-split
    at the token level.
    """
    lines = code.splitlines(keepends=True)
    chunks, buf, buf_tok = [], [], 0

    for line in lines:
        line_tok = len(enc.encode(line))

        # A single line exceeds the limit -> force-split at token level
        if line_tok > max_tokens:
            if buf:
                chunks.append("".join(buf))
                buf, buf_tok = [], 0
            ids = enc.encode(line)
            for i in range(0, len(ids), max_tokens):
                chunks.append(enc.decode(ids[i:i + max_tokens]))
            continue

        # Adding this line would exceed the limit -> flush the buffer
        if buf and buf_tok + line_tok > max_tokens:
            chunks.append("".join(buf))
            buf, buf_tok = [], 0

        buf.append(line)
        buf_tok += line_tok

    if buf:
        chunks.append("".join(buf))
    return chunks


def split_code_by_tokens(code, work_dir, chunk_tokens=40000, out_dir="code_chunks",
                          prefix="segment", encoding_name="cl100k_base"):
    """Split code by token length.

    Returns: (should_split, first_chunk, remaining_paths, c_codes)
      - should_split    : whether splitting was needed (bool)
      - first_chunk     : the first chunk (the whole code if no split)
      - remaining_paths : list of file paths where chunks 2..N were written
      - c_codes         : list of the contents of chunks 2..N (same order as remaining_paths)
    """
    enc = get_encoder(encoding_name)
    chunks = chunk_by_tokens(code, chunk_tokens, enc)

    # No split needed
    if len(chunks) <= 1:
        return False, code, [], []

    # Write chunks 2..N to disk and collect their paths and contents
    out_dir = f"{work_dir}/{out_dir}"
    os.makedirs(out_dir, exist_ok=True)
    remaining_paths = []
    c_codes = []
    for i, chunk in enumerate(chunks[1:], start=2):
        path = os.path.join(out_dir, f"{prefix}_part{i}.c")
        with open(path, "w", encoding="utf-8") as f:
            f.write(chunk)
        remaining_paths.append(path)
        c_codes.append(chunk)

    # Append a truncation notice so downstream readers know the code continues
    first_chunk = chunks[0] + f"\n\n// (... the code is truncated. The remaining code is in {remaining_paths})\n"

    return True, first_chunk, remaining_paths, c_codes


def translate_llm(convert_element, one_unit, rust_path, interface):
    
    llm_interface = interface.llm_interface
    output_max = llm_interface.output_max

    target_path = interface.target_path
    work_dir = interface.work_dir
    build_path = interface.build_path
    rust_build_path = interface.rust_build_path
    run_test_path = interface.run_test_path
    run_all_path = interface.run_all_path
    dep_json_path = interface.dep_json_path
    meta_dir = interface.meta_dir
    div_meta_dir = interface.div_meta_dir
    rust_output_dir = interface.rust_output_dir
    database_dir = interface.database_dir
    original_dir = interface.original_target_dir
    target_dir = interface.target_dir
    is_program_path = interface.is_program_path
    target = interface.target
    user_id = interface.user_id

    # set the initial prompt
    prompt = []    
    add_prompt = [] # Carry-over prompt
    read_prompt = None

    prompt.extend(["Now we have the goal to translate memory-vulnerable C code into memory-safe Rust code to enhance overall security. ",
                "Please translate the following C code segment into Rust.", # without using unsafe.",
                "During the translation process, please strictly apply all the specified rules below and provide only the results that follow the rules.",
                "",
                "## Translation rules:",
                #"- All symbols referenced by the C code below (constants, types, macros, helper functions, global variables) are already defined in either bindings.rs (generated by build.rs) or the existing lib.rs. Use them directly by name and never redefine them.",
                "- Translate all code in the C code segment below into safe Rust. All symbols that are referenced within the C code segment below but whose definition exists outside the segment (constants, macros, helper functions, global variables, type definitions defined in other segments) are already defined in either bindings.rs or the existing lib.rs. Use them directly by name and never redefine them.",
                "- Distinguish between definitions and declarations in the C code segment:",
                "    - Definitions (MUST translate): translate every construct in the C segment that carries content of its own — i.e. anything with a body or an initializer.",
                #"    - Definitions (MUST translate): C code that has a body, including enum and struct/union definitions with their members, and conditional typedefs inside #ifdef/#ifndef blocks — not only functions.",
                "    - Declarations (do NOT translate): function prototypes without body, forward declarations without braces (e.g. struct Foo;, typedef struct Foo Foo;), and extern declarations without initializer. Do NOT invent an implementation for it by yourself.",
                "- Please provide complete code without any omitted sections or placeholders, because the code will be directly copied and pasted it for execution. Do NOT use comments like \"// Implementation omitted for brevity\" or similar.",
                #"- Avoid using unsafe, and achieve equivalent functionality by using the Rust standard library or crates safely.", 
                #"- Please never use unsafe, raw pointers, or manual memory management.",
                "- Please do not use unsafe, raw pointers, or manual memory management as much as possible.", 
                "    - EXCEPTION: Permit minimal unsafe blocks strictly limited to the following two categories.",
                "          - Stub implementation of the FFI boundary functions (specified in ## FFI boundary functions below): These C functions are being replaced by Rust. These are declared as extern C fn with #[unsafe(no_mangle)] so that C code can call the Rust replacement. Stub implementation of the FFI boundary functions MUST remain unchanged until you are explicitly instructed to replace them with the actual implementation. Do NOT implement the actual logic of it.", # Note that even inside FFI boundary functions, extract logic into safe Rust helper functions.",
                "          - Global variables: C global variables shared across the boundary, accessed through unsafe extern C static declarations with getter and setter functions.",
                #"          - cfg attribute flags: Conditional compilation flags registered as cargo rustc cfg, used with #[cfg(has_FLAG_NAME)] attributes.",
                #"          - Independent constant macros: C macro constants generated by bindgen in bindings.rs, used directly by name without redefinition.",
                "    - For everything else, write in safe Rust without calling C functions through FFI.",
                #"- Please achieve equivalent functionality by providing the same logical operations using safe Rust patterns, not by replicating the exact C API.",
                "- When translating C raw pointers to Rust, use appropriate safe representations such as references, slices, Option, Box, Rc, Arc, Vec, etc.",
                #"- Do not add any extra functions or other data, and only write the Rust equivalent of the original C source code.",
                "- After translating functions to Rust, please add the prefix \"rust_\" to all function names.",         
                "- Other than functions, keep the names of variables, constants, and data types the same as the original, but adapt their types and implementations to ensure memory safety.", # , including the same case for uppercase and lowercase letters #"- Keep the names of all translation elements except the functions, such as data types, variables, and macro functions, exactly the same as the original, including the same case for uppercase and lowercase letters. ",
                # "- When writing functions, first add 'use tracing::info;' at the top of the file, then add log outputs using the info!() macro in each function so that the values of the arguments and the return values can be traced as follows:",
                # "    - Just after function entry, log the argument values with: \"Enter_log::: variable_name1 = value1, variable_name2 = value2, ...\", and just before function exit, log the return values with: \"Return_log::: variable_name1 = value1, variable_name2 = value2, ...\"",
                # "    - If an argument or return value is a pointer, please log its value rather than its address.",
                # "    - If there are no arguments or return values, you do not need to include an info!() log output.",
                # "    - For any argument or return value that does not implement the Debug trait (and thus cannot be used with the tracing crate), log its value as <unprintable>.",
                # "- To ensure that the tracing crate can be used, please implement the Debug trait for all data types defined in the code that are used as function argument or return types.",
                #"- For strings that are treated as literal strings in the original C code, ensure that in the translated Rust code these strings are identical for equality checks -this includes preserving case sensitivity (uppercase/lowercase) and making sure every single character matches perfectly. Similarly, any strings originally printed via printf should be translated into Rust exactly as they appear in the original C code.",             
                #"- If there is a file inclusion directive (e.g., #include \"header_a.h\"), assume that a corresponding module has been created (e.g., header_a.h -> header_a_h), and use a wildcard to import the required items from the module by writing use crate::header_a_h::*;.",        
                #"- Convert the function inputs and outputs to ensure equivalence between the original C code and the Rust code.",
                "- Perform the translation strictly within the scope of the C code provided below. Even if you are aware of the entire program, do not add or extend code beyond the C source code shown below.",
                "- The \"rust_code\" field will be appended to the existing lib.rs, so please contain ONLY the newly translated code corresponding to the C code shown below. Do NOT repeat any code already present in the existing lib.rs.",
                #"- If unsafe is required to call external functions, as much as possible, use the Rust standard library or crates to achieve equivalent functionality in a safe manner.",
                #"- Declare all items (structs, enums, functions, constants, etc.) with pub so they can be imported from other modules.",
                "- Define all functions and types at the top level. Avoid using the mod keyword for module partitioning and avoid defining methods for types (within impl blocks).", #"- Define all functions and types at the top level without using the mod keyword.",
                "- When representing backslashes as byte literals, escape the backslash twice in the source code, and also escape it again in the byte literal, resulting in four backslashes (double backslashes).",
                "- When representing backslashes as character literals, escape the backslash once in the source code and again in the character literal, resulting in two backslashes.",
                "- About functions, when translating C functions to Rust, please implement all functions as standalone functions without using Rust's methods or impl blocks.",
    ])


    target_function = get_target_function(one_unit, target_path)

    prompt, sole_prompt = get_context_prompt('divided_type', prompt, one_unit, dep_json_path, is_program_path, 
                                              original_dir, meta_dir, div_meta_dir, rust_output_dir, build_path, target, user_id)
    add_prompt.extend(sole_prompt)

    if len(target_function) != 0:
        prompt.extend([
            "- For the following entry point functions within the FFI boundary functions, make them callable from C code as they will be called directly from C code as entry points.",
            "- For the entry point functions, if a stub implementation already exists, delete the stub implementation first, then write the actual implementation as a complete replacement of it.",
            "- If the entry point function is named 'main', its Rust function name should be made unique by appending _main to the filename (stripped of its extension, with non-alphanumeric characters replaced by _). If a collision still occurs, prefix the parent directory name.",
            "- IMPORTANT: This Rust code is a LIBRARY, not a standalone binary. You MUST NEVER define a function named main or fn main() on the Rust side. The Rust 'main' entry-point functions are called from the C-side main functions as library functions.",
        ])

        prompt.extend(["  - Entry point functions:"])
        for func in target_function:
            prompt.extend([f"    - {func}"])

    functions = parse_function_info(target_path, work_dir, user_id)
    prompt.extend(["\n## FFI boundary functions:"])
    for func in functions:
        prompt.append(f"   - {func['name']} (from {func['file_path']}, line {func['start_line']})")
    
    c_code = get_unit_code(one_unit)
    c_path = f"{database_dir}/tmp.c"
    write_file(c_path, c_code)
    
    chunk_tokens = 40000
    lined_code = get_lined_code(c_path, database_dir, chunk_tokens=False)
    should_split, first_chunk, remaining_paths, c_codes = split_code_by_tokens(lined_code, work_dir, chunk_tokens)

    prompt.extend(["\n## Response format", "In summary, please respond in the following JSON format:"]) 
    prompt.extend([translate_template])

    if should_split is False:
        c_code = get_lined_code(c_path, database_dir, chunk_tokens)
        prompt.extend([
            f"\n## C code segment:", 
            c_code
        ])

    else:
        prompt.extend([
            f"\n## C code segment:", 
            f"The C code segment was too long, so it has been split into multiple parts.", 
            f"The first part is below.",
            f"The remaining parts are stored at paths {remaining_paths}, so please read those files one by one, in order using read_data mode before proceeding with the translation.",
            #f"After this, I will send the fragment information. Until I am done, do not respond with anything other than the JSON answer {{\"status\": \"ok\"}}.",
            "",
            first_chunk, 
        ])
        c_code = first_chunk
        """
        prompt_snapshot = prompt.copy()
        prompt.extend(["\n## Response format", 
                        f"After this, I will send the fragment information. Until I am done, do not respond with anything other than the JSON answer {{\"status\": \"ok\"}}."]) 
        #prompt.extend([translate_template])

        rsp_json = ask_llm(prompt, "init", llm_interface)

        for i in range(len(remaining_paths)):
            path = remaining_paths[i]
            code = c_codes[i]
            remaining_prompt = []
            remaining_prompt.extend([
            f"## C code segment of {path}:", 
                f"The subsequent part is below.",
                f"Until I am done, do not respond with anything other than the JSON answer {{\"status\": \"ok\"}}.",
                "",
                code, 
            ])
            rsp_json = ask_llm(remaining_prompt, "continue", llm_interface)

        prompt = prompt_snapshot.copy()

        prompt.extend(["\n## Response format", "In summary, please respond in the following JSON format:"]) 
        prompt.extend([translate_template])
        """

    # Display the previous read_data
    if read_prompt is not None:
        prompt.extend(["", "## Response to the previous request:"])
        prompt.extend(read_prompt)
        read_prompt = None # Initialization

    rust_code = get_lined_code(rust_path, work_dir)
    prompt.extend([f"\n## Existing code already in {rust_path}:",
    rust_code
    ])

    prompt.extend(["", "## Directory structure of the translated Rust program:"])  
    directory_structure = get_dir_struct("translation", work_dir, None) 
    prompt.extend([directory_structure, ""])

    prompt.extend(["", "## Module structure of the Rust program:"])
    structure = get_cargo_modules(rust_output_dir)
    prompt.extend([structure, ""])

    
    global current_c_block_end
    current_c_block_end = 1 #0

    ref_files = []
    ongoing_flag = False
    no_omission = None
    rsp_json = {}
    current_block_complete = None
    refined_completed = None
    ongoing_count = 0
    read_prompt = None
    exp_data = {}

    init_rust_end = count_file_lines(rust_path) + 1

    while (1):
        if ongoing_flag:
            total_end = count_file_lines(c_path) 
            if current_c_block_end == total_end:
                break
            print("Keep going to receive Rust code.")

            prompt = [f"Please translate the remaining code from after line {current_c_block_end} as shown below. Do NOT translate any code before line {current_c_block_end}.",
                    "Keep following the translation rules.",
                    "",
                    "## Translation rules:",
                    #"- All symbols referenced by the C code below (constants, types, macros, helper functions, global variables) are already defined in either bindings.rs (generated by build.rs) or the existing lib.rs. Use them directly by name and never redefine them.",
                    "- Translate all code in the C code segment below into safe Rust. All symbols that are referenced within the C code segment below but whose definition exists outside the segment (constants, macros, helper functions, global variables, type definitions defined in other segments) are already defined in either bindings.rs or the existing lib.rs. Use them directly by name and never redefine them.",
                    "- Distinguish between definitions and declarations in the C code segment:",
                    "    - Definitions (MUST translate): translate every construct in the C segment that carries content of its own — i.e. anything with a body or an initializer.",
                    #"    - Definitions (MUST translate): C code that has a body, including enum and struct/union definitions with their members, and conditional typedefs inside #ifdef/#ifndef blocks — not only functions.",
                    "    - Declarations (do NOT translate): function prototypes without body, forward declarations without braces (e.g. struct Foo;, typedef struct Foo Foo;), and extern declarations without initializer. Do NOT invent an implementation for it by yourself.",
                    "- Do NOT include any omissions or simplification in your answer code.", 
                    #"- Please do not overlap the c_block_start and c_block_end values with previous responses",
                    #"- Only perform the Rust translation within the range of the original C code initially presented. Do not add any new functions or extensions, and translate only the provided C code.", #"When the translation is complete, set 'ongoing': false.",
                    # "- Perform the translation corresponding to the initially presented C original code. Even if you know the entire program, do not add or extend code beyond the presented C source code.",
                    "- Please do not use unsafe, raw pointers, or manual memory management as much as possible.",
                    "    - EXCEPTION: Permit minimal unsafe blocks strictly limited to the following two categories.",
                    "                  1. Stub implementation of the FFI boundary functions (specified in ## FFI boundary functions below): These C functions are being replaced by Rust. These are declared as extern C fn with #[unsafe(no_mangle)] so that C code can call the Rust replacement. Stub implementation of the FFI boundary functions MUST remain unchanged until you are explicitly instructed to replace them with the actual implementation. Do NOT implement the actual logic of it.", # Note that even inside FFI boundary functions, extract logic into safe Rust helper functions.",
                    "                  2. Global variables: C global variables shared across the boundary, accessed through unsafe extern C static declarations with getter and setter functions.",
                    #"                  3. cfg attribute flags: Conditional compilation flags registered as cargo rustc cfg, used with #[cfg(has_FLAG_NAME)] attributes.",
                    #"                  4. Independent constant macros: C macro constants generated by bindgen in bindings.rs, used directly by name without redefinition.",
                    "    - For everything else, write in safe Rust without calling C functions through FFI.",
                    #"- Do not use unsafe or raw pointers. Instead, please use safe Rust types and operations including custom structs, enums, Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.", 
                    #"- Avoid using unsafe, and use the Rust standard library or crates to achieve equivalent functionality in a safe manner.",
                    #"- When translating C raw pointers to Rust, use appropriate safe representations such as references, slices, Option, Box, Rc, Arc, etc.",
                    #"- Declare all items (structs, enums, functions, constants, etc.) with pub (public) so they can be imported from other modules.", # This also applies to methods.
                    "- Define all functions and types at the top level. Avoid using the mod keyword for module partitioning and avoid defining methods for types (within impl blocks).", #"- Define all functions and types at the top level without using the mod keyword.",
                    f"- The translated code content should be ready to run as-is, without omissions, and should include entire units such as functions or data types, starting from lines without indentation.", # Ensure the code is complete without omission and includes entire units like functions or data types that can be parsed by ctags.
                    "- When representing backslashes as byte literals, escape the backslash twice in the source code, and also escape it again in the byte literal, resulting in four backslashes (double backslashes).",
                    "- When representing backslashes as character literals, escape the backslash once in the source code and again in the character literal, resulting in two backslashes.",
                    "- After translating functions to Rust, please add the prefix \"rust_\" to all function names.",
                    # f"Continue providing the code in a format that can be parsed by json.loads(), within {output_max} tokens, and record it in the value of the 'rust_code' key in JSON format.", # Return the code in JSON format that can be parsed by json.loads()
                    # "Ensure that escape characters (e.g., newlines, double quotes, backslashes) are appropriately escaped in the JSON response.",
                    # f"Continue encoding the code in Base64, within {output_max} tokens, and write it in the value of the 'rust_code' key in JSON format.",
                    f"- Continue providing the code, keeping it within {output_max} tokens, and record it in the value of the 'rust_code' key in JSON format.",
                    "- If your answer is likely to exceed the token limit, please split the response into multiple parts.", # but it's omitted for brevity as it would exceed the token limit.
                    "- If the translated code is the last part, set the 'ongoing' key to a boolean value of False. If there is remaining code, set the 'ongoing' key to a boolean value of True.",
                    # "Ensure that the Rust code provided in one response does not interrupt function or struct definitions midway.",
                    #"- Instead of trying to strictly reproduce the exact details of the original code, aim to achieve similar functionality using idiomatic Rust approaches.",
                    #"- Ensure that the function inputs and outputs match between the original C code and the translated Rust code (maintain equivalence).",
                    # "Do not add a main function unless the original C code contains one.",
                    # f"I'm saying this again because it's important: Do not try to convert everything at once. Keep the code included in 'rust_code' within {output_max} tokens for one part. Please strictly follow this instruction.",
                    ]


            if len(target_function) != 0:  #len(target_function) == 0:
                prompt.extend([
                    "- For the following entry point functions within the FFI boundary functions, make them callable from C code as they will be called directly from C code as entry points.",
                    "- For the entry point functions, if a stub implementation already exists, delete the stub implementation first, then write the actual implementation as a complete replacement of it.",
                    "- If the entry point function is named 'main', its Rust function name should be made unique by appending _main to the filename (stripped of its extension, with non-alphanumeric characters replaced by _). If a collision still occurs, prefix the parent directory name.",
                    "- IMPORTANT: This Rust code is a LIBRARY, not a standalone binary. You MUST NEVER define a function named main or fn main() on the Rust side. The Rust 'main' entry-point functions are called from the C-side main functions as library functions.",
                ])

                prompt.extend(["  - Entry point functions:"])
                for func in target_function:
                    prompt.extend([f"    - {func}"])
            
            prompt.extend(["\n## FFI boundary functions:"])
            for func in functions:
                prompt.append(f"   - {func['name']} (from {func['file_path']}, line {func['start_line']})") # lines {func['start_line']}-{func['end_line']})")
            
            prompt.extend(["\n## Response format", "In summary, please respond in the following JSON format:"]) 
            prompt.extend([translate_template])

            """
            c_code = get_lined_specific_code(database_dir, c_path, current_c_block_end, total_end)
            prompt.extend([
                f"\n## C source code):", # (lines {current_c_block_end} - {total_end} in the file {c_path}
                c_code
            ])
            """

        # Display the previous read_data mode
        if read_prompt is not None:
            prompt.extend(["", "## Response to the previous request:"])
            prompt.extend(read_prompt)
            read_prompt = None # Initialization

        exp_data['repair_count'] = 0
        exp_data['phase'] = 'convert'
        if ongoing_count == 0:

            if INSERT_FILES:
                if len(ref_files) != 0:
                    rust_ref_files = []
                    for ref in ref_files:
                        ref_rust_path = get_rust_path(ref, map_path)
                        ref_base_path = remove_base_path(ref_rust_path, rust_output_dir)
                        rust_ref_files.append(ref_base_path)

                    history_path = []
                    base_rust_path = remove_base_path(rust_path, rust_output_dir)
                    history_path.append(base_rust_path)
                    history_path.extend(rust_ref_files)
                    
                    rsp_json = ask_llm(prompt, history_path, llm_interface)
                    if 'c_block_end' in rsp_json:
                        c_block_end = rsp_json['c_block_end']
                        current_c_block_end = c_block_end 
                        
                else:
                    rsp_json = ask_llm(prompt, "init", llm_interface)

            else:
                rsp_json = ask_llm(prompt, "init", llm_interface)

        else:
            if not W_O_DEP:
                prompt.extend(["", "## Module structure of the target Rust program:"])
                structure = get_cargo_modules(rust_output_dir)
                prompt.extend([structure, ""])
                

            rsp_json = ask_llm(prompt, "continue", llm_interface) 
            if 'c_block_end' in rsp_json:
                c_block_end = rsp_json['c_block_end']
                current_c_block_end = c_block_end 

        if 'c_block_end' in rsp_json:
            c_block_end = rsp_json['c_block_end']
            current_c_block_end = c_block_end 

        toml_submit = False
        build_submit = False

        if 'ongoing' in rsp_json:
            ongoing_flag = rsp_json['ongoing']
        
        if 'mode' in rsp_json:
            mode = rsp_json['mode']

            if mode == "write_rust":
                if 'rust_code' in rsp_json:
                    append_rust_path(rust_path, rsp_json['rust_code'])
                
                if 'toml' in rsp_json:
                    if rsp_json['toml'] is not None:
                        # Here, rewrite the entire Cargo.toml
                        cargo_toml_path = rust_output_dir + "/" + "Cargo.toml"

                        existing_data = load_toml_file(cargo_toml_path)
                        merge_toml_json(existing_data, rsp_json['toml']) # Overwrite existing data with new JSON data
                        updated_toml_data = toml.dumps(existing_data)  # Convert the overwritten data to TOML format
                        write_toml_file(updated_toml_data, cargo_toml_path) # Write TOML data to file

                if 'no_omission' in rsp_json:
                    no_omission = rsp_json['no_omission']
                    if no_omission is False:
                        ongoing_flag = True

                if 'current_block_complete' in rsp_json:
                    current_block_complete = rsp_json['current_block_complete']
                    if current_block_complete is False:
                        ongoing_flag = True

            elif mode == "read_data":
                read_prompt = []
                sum_target_list = []
                sum_slice_list = []

                if 'target_files' in rsp_json and rsp_json['target_files'] is not None:
                    target_list = rsp_json['target_files']
                    if not isinstance(target_list, list):
                        target_list = [target_list]
                    sum_target_list.extend(target_list)

                if 'file_slices' in rsp_json and rsp_json['file_slices'] is not None:
                    slice_list = rsp_json['file_slices']
                    if not isinstance(slice_list, list):
                        slice_list = [slice_list]
                    sum_slice_list.extend(slice_list)

                read_prompt = ["The content obtained in read_data mode is as follows.", ""]
            
                for see_path in sum_target_list:
                    file_code = get_lined_code(see_path, work_dir)
                    read_prompt.extend([f"- Content of the file {see_path}:"])  
                    if len(file_code) == 0:
                        file_code = f"Line 1 [0]: [This {see_path} file is currently empty and contains no content. *** STOP *** Do not use read_data mode anymore.]"
                    read_prompt.extend([f'{file_code}\n'])

                for see_item in sum_slice_list:
                    file_code = get_lined_specific_code(database_dir, see_item['file_path'], see_item['start_line'], see_item['end_line'])
                    read_prompt.extend([f"- Content of {see_item['start_line']} - {see_item['end_line']} lines in the file {see_item['file_path']}:"]) 
                    read_prompt.extend([f'{file_code}\n'])
                    
                
        iteration_dict[rust_path] = 1

        if not ongoing_flag:
            break

        ongoing_count += 1


    last_rust_end = count_file_lines(rust_path)

    copy_file(rust_path, f"{database_dir}/unrefined.rs")
    refined_rust_code = read_specific_lines(rust_path, 1, init_rust_end-1)
    #refined_rust_code = get_lined_specific_code(rust_path, 1, init_rust_end-1)
    delete_file(rust_path)
    write_file(rust_path, refined_rust_code)

    prompt = []
    prompt.extend([f"The following Rust code (lines {init_rust_end}-{last_rust_end} in {rust_path}) was generated from the original C code.",
                   "Please refine this code to be more idiomatic Rust, focusing on safety and aligning it with Rust's principles rather than making it a direct translation from C.",
                   #"Please start writing the complete code from c_block_start = 1, even if no changes are made to the code."
                   ])
    
    prompt.extend(["", "## Response rules",  
                    #"- All symbols referenced by the C code below (constants, types, macros, helper functions, global variables) are already defined in either bindings.rs (generated by build.rs) or the existing lib.rs. Use them directly by name and never redefine them.",
                    "- Translate all code in the C code segment below into safe Rust. All symbols that are referenced within the C code segment below but whose definition exists outside the segment (constants, macros, helper functions, global variables, type definitions defined in other segments) are already defined in either bindings.rs or the existing lib.rs. Use them directly by name and never redefine them.",
                    "- Distinguish between definitions and declarations in the C code segment:",
                    "    - Definitions (MUST translate): translate every construct in the C segment that carries content of its own — i.e. anything with a body or an initializer.",
                    #"    - Definitions (MUST translate): C code that has a body, including enum and struct/union definitions with their members, and conditional typedefs inside #ifdef/#ifndef blocks — not only functions.",
                    "    - Declarations (do NOT translate): function prototypes without body, forward declarations without braces (e.g. struct Foo;, typedef struct Foo Foo;), and extern declarations without initializer. Do NOT invent an implementation for it by yourself.",
                    #"- In particular, do NOT use unsafe or raw pointers. Instead, please use appropriate safe alternatives like Box, Arc, Vec, and others.",
                    #"- Do not use unsafe or raw pointers. Instead, please use safe Rust types and operations including custom structs, enums, Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.", #"- NEVER use unsafe or raw pointers. Only use safe Rust types and operations: Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.",
                    "- Please do not use unsafe, raw pointers, or manual memory management as much as possible.",
                    "    - EXCEPTION: Permit minimal unsafe blocks strictly limited to the following two categories.",
                    "                  1. Stub implementation of the FFI boundary functions (specified in ## FFI boundary functions below): These C functions are being replaced by Rust. These are declared as extern C fn with #[unsafe(no_mangle)] so that C code can call the Rust replacement. Stub implementation of the FFI boundary functions MUST remain unchanged until you are explicitly instructed to replace them with the actual implementation. Do NOT implement the actual logic of it.", # Note that even inside FFI boundary functions, extract logic into safe Rust helper functions.",
                    "                  2. Global variables: C global variables shared across the boundary, accessed through unsafe extern C static declarations with getter and setter functions.",
                    #"                  3. cfg attribute flags: Conditional compilation flags registered as cargo rustc cfg, used with #[cfg(has_FLAG_NAME)] attributes.",
                    #"                  4. Independent constant macros: C macro constants generated by bindgen in bindings.rs, used directly by name without redefinition.",
                    "    - For everything else, write in safe Rust without calling C functions through FFI.",
                    #"- For global variables, prefer const for compile-time constants. For runtime mutable global variables, do NOT use static mut - instead, choose an appropriate type based on the purpose (atomic types, Cell types, OnceCell/Lazy, Mutex/RwLock, etc). Consider whether global state is truly necessary, and if it is, try grouping related variables into a struct.", #"- For global variables, do NOT use static mut. Instead, choose an appropriate type based on the purpose of use. First, consider whether global state is truly necessary - prefer local variables when possible. If global state is necessary, consider grouping related variables into a struct where possible, then select an appropriate type based on usage from options such as atomic types, const, Cell types, OnceCell/Lazy, Mutex/RwLock, etc.", #"- For global variables, do NOT use static mut. Instead, choose an appropriate type based on the purpose of use and whether it will be accessed across threads, such as const, Cell types, OnceCell/Lazy, atomic types, or Mutex/RwLock.",
                    "- Maintain complete functional equivalence with the original C implementation while leveraging Rust's safety features. Do not omit or simplify any functionality from the original code.",
                    "- It is important to note that you should NOT simplify the code for safety. Do NOT include any omissions or simplification in your answer code.", 
                    #"- However, the primary goal is to fully implement the original C functionality without simplification. If the implementation cannot be achieved without using unsafe or other C-based constructs, their use is permitted.",
                    #"- Declare all items (structs, enums, functions, constants, etc.) with pub (public) so they can be imported from other modules.",
                    "- Define all functions and types at the top level. Avoid using the mod keyword for module partitioning and avoid defining methods for types (within impl blocks).", #
                    # "- When writing functions, first add 'use tracing::info;' at the top of the file, then add log outputs using the info!() macro in each function as follows so that the values of the arguments and the return values can be traced:",
                    # "    - Just after function entry, log the argument values with: \"Enter_log::: variable_name1 = value1, variable_name2 = value2, ...\", and just before function exit, log the return values with: \"Return_log::: variable_name1 = value1, variable_name2 = value2, ...\"",
                    # "    - If an argument or return value is a pointer, please log its value rather than its address.",
                    # "    - If there are no arguments or return values, you do not need to include an info!() log output.",
                    # "    - For any argument or return value that does not implement the Debug trait (and thus cannot be used with the tracing crate), log its value as <unprintable>.",
                    # "- To ensure that the tracing crate can be used, please implement the Debug trait for all data types defined in the code that are used as function argument or return types.",
                    #"- For strings that are treated as literal strings in the original C code, ensure that in the translated Rust code these strings are identical for equality checks -this includes preserving case sensitivity (uppercase/lowercase) and making sure every single character matches perfectly. Similarly, any strings originally printed via printf should be translated into Rust exactly as they appear in the original C code.",             
                    f"- To avoid hitting the token limit, keep the JSON data included in one response within {output_max} tokens, and provide only the first part of the JSON data with clear separation for now, even if the response will be split into multiple parts.", # If it's a long response,
                    f"- If the response is split into multiple parts and there is still remaining JSON data, write a boolean value of True for the 'ongoing' key. If the JSON data is the final part, write a boolean value of False for the 'ongoing' key.",
                ])
    
    prompt.extend(sole_prompt)

    prompt.extend(["\n## FFI boundary functions:"])
    for func in functions:
        prompt.append(f"   - {func['name']} (from {func['file_path']}, line {func['start_line']})") # lines {func['start_line']}-{func['end_line']})")
    

    rust_code = get_lined_specific_code(database_dir, f"{database_dir}/unrefined.rs", init_rust_end, last_rust_end)
    prompt.extend([
        f"\n## Rust source code ({rust_path}):",
        rust_code
    ])

    chunk_tokens = 40000
    lined_code = get_lined_code(c_path, database_dir, chunk_tokens=False)
    should_split, first_chunk, remaining_paths, c_codes = split_code_by_tokens(lined_code, work_dir, chunk_tokens)

    prompt.extend(["\n## Response format", "Please provide your response in the following JSON format:"]) 
    prompt.extend([refine_template])

    if should_split is False:
        prompt.extend([
            f"\n## C code segment:", 
            c_code
        ])

    else:
        prompt.extend([
            f"\n## C code segment:", 
            f"The C code segment was too long, so it has been split into multiple parts.", 
            f"The first part is below.",
            f"The remaining parts are stored at paths {remaining_paths}, so please read those files one by one, in order using read_data mode before proceeding with the translation.",
            #f"After this, I will send the fragment information. Until I am done, do not respond with anything other than the JSON answer {{\"status\": \"ok\"}}.",
            "",
            first_chunk, 
        ])
        c_code = first_chunk

        """
        prompt_snapshot = prompt.copy()

        prompt.extend(["\n## Response format", 
                        f"After this, I will send the fragment information. Until I am done, do not respond with anything other than the JSON answer {{\"status\": \"ok\"}}."]) 
        #prompt.extend([translate_template])

        rsp_json = ask_llm(prompt, "init", llm_interface)

        for i in range(len(remaining_paths)):
            path = remaining_paths[i]
            c_code = c_codes[i]
            remaining_prompt = []
            remaining_prompt.extend([
            f"## C code segment of {path}:", 
                f"The subsequent part is below.",
                f"Until I am done, do not respond with anything other than the JSON answer {{\"status\": \"ok\"}}.",
                "",
                c_code, 
            ])
            rsp_json = ask_llm(remaining_prompt, "continue", llm_interface)

        prompt = prompt_snapshot.copy()

        prompt.extend(["\n## Response format", "Please provide your response in the following JSON format:"]) 
        prompt.extend([refine_template])
        """

    # Display the previous read_data mode
    if read_prompt is not None:
        prompt.extend(["", "## Response to the previous request:"])
        prompt.extend(read_prompt)
        read_prompt = None # Initialization

    prompt.extend(["", "## Directory structure of the translated Rust program:"])  
    directory_structure = get_dir_struct("translation", work_dir, None)  #rust_output_dir)
    prompt.extend([directory_structure, ""])

    prompt.extend(["", "## Module structure of the Rust program:"])
    structure = get_cargo_modules(rust_output_dir)
    prompt.extend([structure, ""])

    save_prompt = prompt.copy()

    ref_files = []
    ongoing_flag = False
    #ongoing_count = 0
    rust_block_start = None
    rust_block_end = None
    current_c_block_end = 1 #0

    while (1):
        if ongoing_flag:
            print("Keep going to receive Rust code.")

            prompt = [f"Please provide only the refined code corresponding to current code lines {current_c_block_end} onwards. Do NOT include refined code for current code lines before {current_c_block_end}.",
                    "Keep following the rules below.",
                    "",
                    "## Response rules",
                    #"- All symbols referenced by the C code below (constants, types, macros, helper functions, global variables) are already defined in either bindings.rs (generated by build.rs) or the existing lib.rs. Use them directly by name and never redefine them.",
                    "- Translate all code in the C code segment below into safe Rust. All symbols that are referenced within the C code segment below but whose definition exists outside the segment (constants, macros, helper functions, global variables, type definitions defined in other segments) are already defined in either bindings.rs or the existing lib.rs. Use them directly by name and never redefine them.",
                    "- Distinguish between definitions and declarations in the C code segment:",
                    "    - Definitions (MUST translate): translate every construct in the C segment that carries content of its own — i.e. anything with a body or an initializer.",
                    #"    - Definitions (MUST translate): C code that has a body, including enum and struct/union definitions with their members, and conditional typedefs inside #ifdef/#ifndef blocks — not only functions.",
                    "    - Declarations (do NOT translate): function prototypes without body, forward declarations without braces (e.g. struct Foo;, typedef struct Foo Foo;), and extern declarations without initializer. Do NOT invent an implementation for it by yourself.",
                    #"- Please do not overlap the c_block_start and c_block_end values with previous responses",
                    #"- Only perform the Rust translation within the range of the original C code initially presented. Do not add any new functions or extensions, and translate only the provided C code.", #"When the translation is complete, set 'ongoing': false.",
                    #"- Do not use unsafe or raw pointers. Instead, please use safe Rust types and operations including custom structs, enums, Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.", #"- NEVER use unsafe or raw pointers. Only use safe Rust types and operations: Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.",
                    "- Please do not use unsafe, raw pointers, or manual memory management as much as possible.",
                    "    - EXCEPTION: Permit minimal unsafe blocks strictly limited to the following two categories.",
                    "                  1. Stub implementation of the FFI boundary functions (specified in ## FFI boundary functions below): These C functions are being replaced by Rust. These are declared as extern C fn with #[unsafe(no_mangle)] so that C code can call the Rust replacement. Stub implementation of the FFI boundary functions MUST remain unchanged until you are explicitly instructed to replace them with the actual implementation. Do NOT implement the actual logic of it.", # Note that even inside FFI boundary functions, extract logic into safe Rust helper functions.",
                    "                  2. Global variables: C global variables shared across the boundary, accessed through unsafe extern C static declarations with getter and setter functions.",
                    #"                  3. cfg attribute flags: Conditional compilation flags registered as cargo rustc cfg, used with #[cfg(has_FLAG_NAME)] attributes.",
                    #"                  4. Independent constant macros: C macro constants generated by bindgen in bindings.rs, used directly by name without redefinition.",
                    "    - For everything else, write in safe Rust without calling C functions through FFI.",
                    #"- Avoid using unsafe, and use the Rust standard library or crates to achieve equivalent functionality in a safe manner.",
                    #"- When translating C raw pointers to Rust, use appropriate safe representations such as references, slices, Option, Box, Rc, Arc, etc.",
                    #"- Declare all items (structs, enums, functions, constants, etc.) with pub (public) so they can be imported from other modules.", # This also applies to methods.
                    "- Define all functions and types at the top level. Avoid using the mod keyword for module partitioning and avoid defining methods for types (within impl blocks).", #"- Define all functions and types at the top level without using the mod keyword.",
                    # "- Perform the translation corresponding to the initially presented C original code. Even if you know the entire program, do not add or extend code beyond the presented C source code.",
                    "- If your answer is likely to exceed the token limit, please split the response into multiple parts.", # but it's omitted for brevity as it would exceed the token limit.
                    "- When representing backslashes as byte literals, escape the backslash twice in the source code, and also escape it again in the byte literal, resulting in four backslashes (double backslashes).",
                    "- When representing backslashes as character literals, escape the backslash once in the source code and again in the character literal, resulting in two backslashes.",
                    # f"Continue providing the code in a format that can be parsed by json.loads(), within {output_max} tokens, and record it in the value of the 'rust_code' key in JSON format.", # Return the code in JSON format that can be parsed by json.loads()
                    # "Ensure that escape characters (e.g., newlines, double quotes, backslashes) are appropriately escaped in the JSON response.",
                    # f"Continue encoding the code in Base64, within {output_max} tokens, and write it in the value of the 'rust_code' key in JSON format.",
                    f"- Continue providing the code, keeping it within {output_max} tokens, and record it in the value of the 'rust_code' key in JSON format.",
                    f"- The translated code content should be ready to run as-is, without omissions, and should include entire units such as functions or data types, starting from lines without indentation.", # Ensure the code is complete without omission and includes entire units like functions or data types that can be parsed by ctags.
                    "- If the translated code is the last part, set the 'ongoing' key to a boolean value of False. If there is remaining code, set the 'ongoing' key to a boolean value of True.",
                    # "Ensure that the Rust code provided in one response does not interrupt function or struct definitions midway.",
                    #"- Instead of trying to strictly reproduce the exact details of the original code, aim to achieve similar functionality using idiomatic Rust approaches.",
                    #"- Ensure that the function inputs and outputs match between the original C code and the translated Rust code (maintain equivalence).",
                    # "Do not add a main function unless the original C code contains one.",
                    # f"I'm saying this again because it's important: Do not try to convert everything at once. Keep the code included in 'rust_code' within {output_max} tokens for one part. Please strictly follow this instruction.",
                    ]

            prompt.extend(["\n## FFI boundary functions:"])
            for func in functions:
                prompt.append(f"   - {func['name']} (from {func['file_path']}, line {func['start_line']})") # lines {func['start_line']}-{func['end_line']})")
            

            prompt.extend(["\nPlease provide your response in the following JSON format:"])
            prompt.extend([refine_template])

            """
            else:
                lack = ""
                if no_omission is False and current_block_complete is False:
                    lack = "omitted parts and code that does not implement the same functionalities as the original C code"
                elif no_omission is False:
                    lack = "omitted parts"
                elif current_block_complete is False:
                    lack = "code that does not fully implement the same functionalities as the original C code"

                prompt = [f"The previously provided code contains {lack}."]
                prompt.extend([f"Please follow the instructions and rewrite the Rust code without including any {lack}.", ""])

                prompt.extend(["- Please provide your response in the following JSON format:"])
                prompt.extend([refine_template])

                c_block_start = None
                c_block_end = None

                if 'c_block_start' in rsp_json:
                    c_block_start = rsp_json['c_block_start']
                
                if 'c_block_end' in rsp_json:
                    c_block_end = rsp_json['c_block_end']

                
                if c_block_start is not None and c_block_end is not None:
                    c_code = get_lined_specific_code(c_path, c_block_start, c_block_end)
                    prompt.extend([
                        f"\n## C source code ({c_path})（{c_block_start} - {c_block_end} lines）:",
                        c_code
                    ])
                else:
                    c_code = get_lined_code(c_path, work_dir)
                    prompt.extend([
                        f"\n## C source code ({c_path}):",
                        c_code
                    ])


                # c_code = get_lined_code(c_path, work_dir)
                # prompt.extend([
                #     f"\n## C source code ({c_path}):",
                #     c_code
                # ])

                rust_code = get_lined_code(rust_path, work_dir)
                prompt.extend([
                    f"\n## Previously provided Rust code ({rust_path}):", 
                    rust_code
                ])

                prompt.extend(["", "## Directory structure of the translated Rust program:"])  
                directory_structure = get_dir_struct(work_dir)  #rust_output_dir)
                prompt.extend([directory_structure, ""])

                prompt.extend(["", "## Module structure of the Rust program:"])
                structure = get_cargo_modules(rust_output_dir)
                prompt.extend([structure, ""])

                delete_file(rust_path)
                create_file(rust_path)

                #prompt.extend(save_prompt)
            """
            
            exp_data['repair_count'] = 0
            exp_data['phase'] = 'convert'
            
        if rust_block_start is not None and rust_block_end is not None:
            #rust_code = get_lined_specific_code("unrefined.rs", rust_block_start, rust_block_end)
            #rust_code = get_lined_code(f"{database_dir}/unrefined.rs", work_dir)
            rust_code = read_file(f"{database_dir}/unrefined.rs")
            prompt.extend([
                f"\n## Unrefined Rust code:",
                rust_code
            ])
            
        rsp_json = ask_llm(prompt, "continue", llm_interface)
        
        if 'c_block_end' in rsp_json:
            c_block_end = rsp_json['c_block_end']
            current_c_block_end = c_block_end 

        if 'rust_block_start' in rsp_json:
            rust_block_start = rsp_json['rust_block_start']
        
        if 'rust_block_end' in rsp_json:
            rust_block_end = rsp_json['rust_block_end']
            current_c_block_end = rust_block_end 

        toml_submit = False
        build_submit = False

        if 'ongoing' in rsp_json:
            ongoing_flag = rsp_json['ongoing']

        else:
            print("Should include ongoing flag") 
        
        if 'mode' in rsp_json:
            mode = rsp_json['mode']

            if mode == "write_rust":
                if 'rust_code' in rsp_json: 
                    append_rust_path(rust_path, rsp_json['rust_code'])
                
                if 'toml' in rsp_json: 
                    if rsp_json['toml'] is not None:
                        # Here, rewrite the entire Cargo.toml
                        cargo_toml_path = rust_output_dir + "/" + "Cargo.toml"

                        existing_data = load_toml_file(cargo_toml_path)
                        merge_toml_json(existing_data, rsp_json['toml']) # Overwrite existing data with new JSON data
                        updated_toml_data = toml.dumps(existing_data) # Convert the overwritten data to TOML format
                        write_toml_file(updated_toml_data, cargo_toml_path) # Write TOML data to file
                
                if 'no_omission' in rsp_json:
                    no_omission = rsp_json['no_omission']
                    if no_omission is False:
                        ongoing_flag = True

                if 'current_block_complete' in rsp_json:
                    current_block_complete = rsp_json['current_block_complete']
                    if current_block_complete is False:
                        ongoing_flag = True

                if 'refined_completed' in rsp_json:
                    refined_completed = True

            elif mode == "read_data":
                read_prompt = []
                sum_target_list = []
                sum_slice_list = []

                if 'target_files' in rsp_json and rsp_json['target_files'] is not None:
                    target_list = rsp_json['target_files']
                    if not isinstance(target_list, list):
                        target_list = [target_list]
                    sum_target_list.extend(target_list)

                if 'file_slices' in rsp_json and rsp_json['file_slices'] is not None:
                    slice_list = rsp_json['file_slices']
                    if not isinstance(slice_list, list):
                        slice_list = [slice_list]
                    sum_slice_list.extend(slice_list)

                read_prompt = ["The content obtained in read_data mode is as follows.", ""]
            
                for see_path in sum_target_list:
                    file_code = get_lined_code(see_path, work_dir)
                    read_prompt.extend([f"- Content of the file {see_path}:"])  
                    if len(file_code) == 0:
                        file_code = f"Line 1 [0]: [This {see_path} file is currently empty and contains no content. *** STOP *** Do not use read_data mode anymore.]"
                    read_prompt.extend([f'{file_code}\n'])

                for see_item in sum_slice_list:
                    file_code = get_lined_specific_code(database_dir, see_item['file_path'], see_item['start_line'], see_item['end_line'])
                    read_prompt.extend([f"- Content of {see_item['start_line']} - {see_item['end_line']} lines in the file {see_item['file_path']}:"]) 
                    read_prompt.extend([f'{file_code}\n'])
                    

        iteration_dict[rust_path] = 1

        if not ongoing_flag:
            break

        ongoing_count += 1


def translate_llm_minimize(convert_element, one_unit, rust_path, interface):
    
    # Config
    llm_interface = interface.llm_interface
    output_max = llm_interface.output_max

    target_path = interface.target_path
    work_dir = interface.work_dir
    build_path = interface.build_path
    rust_build_path = interface.rust_build_path
    run_test_path = interface.run_test_path
    run_all_path = interface.run_all_path
    dep_json_path = interface.dep_json_path
    meta_dir = interface.meta_dir
    div_meta_dir = interface.div_meta_dir
    rust_output_dir = interface.rust_output_dir
    database_dir = interface.database_dir
    original_dir = interface.original_target_dir
    target_dir = interface.target_dir
    is_program_path = interface.is_program_path
    target = interface.target
    user_id = interface.user_id

    prompt = []    
    add_prompt = [] # Carry-over prompt
    read_prompt = None

    # Set the initial prompt
    prompt.extend(["Now we have the goal to translate memory-vulnerable C code into memory-safe Rust code to enhance overall security. ",
                "Please translate the following C code segment into Rust.", # without using unsafe.",
                "During the translation process, please strictly apply all the specified rules below and provide only the results that follow the rules.",
                "",
                "## Translation rules:",
                #"- All symbols referenced by the C code below (constants, types, macros, helper functions, global variables) are already defined in either bindings.rs (generated by build.rs) or the existing lib.rs. Use them directly by name and never redefine them.",
                "- Translate all code in the C code segment below into safe Rust. All symbols that are referenced within the C code segment below but whose definition exists outside the segment (constants, macros, helper functions, global variables, type definitions defined in other segments) are already defined in either bindings.rs or the existing lib.rs. Use them directly by name and never redefine them.",
                "- Distinguish between definitions and declarations in the C code segment:",
                "    - Definitions (MUST translate): translate every construct in the C segment that carries content of its own — i.e. anything with a body or an initializer.",
                #"    - Definitions (MUST translate): C code that has a body, including enum and struct/union definitions with their members, and conditional typedefs inside #ifdef/#ifndef blocks — not only functions.",
                "    - Declarations (do NOT translate): function prototypes without body, forward declarations without braces (e.g. struct Foo;, typedef struct Foo Foo;), and extern declarations without initializer. Do NOT invent an implementation for it by yourself.",
                "- Please provide complete code without any omitted sections or placeholders, because the code will be directly copied and pasted it for execution. Do NOT use comments like \"// Implementation omitted for brevity\" or similar.",
                #"- Avoid using unsafe, and achieve equivalent functionality by using the Rust standard library or crates safely.", 
                #"- Please never use unsafe, raw pointers, or manual memory management.",
                "- Please do not use unsafe, raw pointers, or manual memory management as much as possible.", 
                "    - EXCEPTION: Permit minimal unsafe blocks strictly limited to the following two categories.",
                "          - The rust_main_<identifier> entry point: declared as pub extern \"C\" fn rust_main_<identifier>() with #[no_mangle] so that C main() can call it. This is the only FFI boundary in this project. Stub implementation MUST remain unchanged until you are explicitly instructed to replace it with the actual implementation. Do NOT implement the actual logic of it.",
                #"                  2. Global variables: C global variables shared across the boundary, accessed through unsafe extern C static declarations with getter and setter functions.",
                #"                  3. cfg attribute flags: Conditional compilation flags registered as cargo rustc cfg, used with #[cfg(has_FLAG_NAME)] attributes.",
                #"                  4. Independent constant macros: C macro constants generated by bindgen in bindings.rs, used directly by name without redefinition.",
                "    - For everything else, write in safe Rust without calling C functions through FFI.",
                #"- Please achieve equivalent functionality by providing the same logical operations using safe Rust patterns, not by replicating the exact C API.",
                "- When translating C raw pointers to Rust, use appropriate safe representations such as references, slices, Option, Box, Rc, Arc, Vec, etc.",
                #"- Do not add any extra functions or other data, and only write the Rust equivalent of the original C source code.",
                "- After translating functions to Rust, please add the prefix \"rust_\" to all function names.",         
                "- Other than functions, keep the names of variables, constants, and data types the same as the original, but adapt their types and implementations to ensure memory safety.", # , including the same case for uppercase and lowercase letters #"- Keep the names of all translation elements except the functions, such as data types, variables, and macro functions, exactly the same as the original, including the same case for uppercase and lowercase letters. ",
                # "- When writing functions, first add 'use tracing::info;' at the top of the file, then add log outputs using the info!() macro in each function so that the values of the arguments and the return values can be traced as follows:",
                # "    - Just after function entry, log the argument values with: \"Enter_log::: variable_name1 = value1, variable_name2 = value2, ...\", and just before function exit, log the return values with: \"Return_log::: variable_name1 = value1, variable_name2 = value2, ...\"",
                # "    - If an argument or return value is a pointer, please log its value rather than its address.",
                # "    - If there are no arguments or return values, you do not need to include an info!() log output.",
                # "    - For any argument or return value that does not implement the Debug trait (and thus cannot be used with the tracing crate), log its value as <unprintable>.",
                # "- To ensure that the tracing crate can be used, please implement the Debug trait for all data types defined in the code that are used as function argument or return types.",
                #"- For strings that are treated as literal strings in the original C code, ensure that in the translated Rust code these strings are identical for equality checks -this includes preserving case sensitivity (uppercase/lowercase) and making sure every single character matches perfectly. Similarly, any strings originally printed via printf should be translated into Rust exactly as they appear in the original C code.",             
                #"- If there is a file inclusion directive (e.g., #include \"header_a.h\"), assume that a corresponding module has been created (e.g., header_a.h -> header_a_h), and use a wildcard to import the required items from the module by writing use crate::header_a_h::*;.",        
                #"- Convert the function inputs and outputs to ensure equivalence between the original C code and the Rust code.",
                "- Perform the translation strictly within the scope of the C code provided below. Even if you are aware of the entire program, do not add or extend code beyond the C source code shown below.",
                "- The \"rust_code\" field will be appended to the existing lib.rs, so please contain ONLY the newly translated code corresponding to the C code shown below. Do NOT repeat any code already present in the existing lib.rs.",
                #"- If unsafe is required to call external functions, as much as possible, use the Rust standard library or crates to achieve equivalent functionality in a safe manner.",
                #"- Declare all items (structs, enums, functions, constants, etc.) with pub so they can be imported from other modules.",
                "- Define all functions and types at the top level. Avoid using the mod keyword for module partitioning and avoid defining methods for types (within impl blocks).", #"- Define all functions and types at the top level without using the mod keyword.",
                "- When representing backslashes as byte literals, escape the backslash twice in the source code, and also escape it again in the byte literal, resulting in four backslashes (double backslashes).",
                "- When representing backslashes as character literals, escape the backslash once in the source code and again in the character literal, resulting in two backslashes.",
                "- About functions, when translating C functions to Rust, please implement all functions as standalone functions without using Rust's methods or impl blocks.",
    ])

    target_function = get_target_function(one_unit, target_path)

    prompt, sole_prompt = get_context_prompt('divided_type', prompt, one_unit, dep_json_path, is_program_path, 
                                              original_dir, meta_dir, div_meta_dir, rust_output_dir, build_path, target, user_id)
    add_prompt.extend(sole_prompt)

    if len(target_function) != 0:
        prompt.extend([
            "- For the following entry point function, translate it as the Rust entry point named 'rust_main_<identifer>'.",
            "- If a stub implementation of rust_main_<identifer> already exists, delete the stub first, then write the actual implementation as a complete replacement.",
            f"  Note that <identifier> is derived from the C source file containing the corresponding main() function and set to be unique across all main() functions, so that each main() maps to a distinct rust_main_<identifier> / rust_main_wrapper_<identifier> pair with no name collisions; however, if there is only one main() function in the project, the <identifier> suffix is entirely omitted.",
            "- When translating the C main function, obtain command-line arguments using std::env::args() in Rust instead of receiving argc/argv from C.",
            "- Do NOT use raw pointers (*const, *mut) for argument handling. Use Vec<String> or &[String] instead.",
            "- IMPORTANT: The entry point function must be #[no_mangle] pub extern \"C\" fn rust_main_<identifer>(). This is the only FFI boundary. All other functions should be pure safe Rust with no extern or #[no_mangle].",
        ])

        prompt.extend(["  - Entry point functions:"])
        for func in target_function:
            prompt.extend([f"    - {func}"])

    functions = parse_function_info(target_path, work_dir, user_id)
    prompt.extend(["\n## FFI boundary functions:"])
    for func in functions:
        prompt.append(f"   - {func['name']} (from {func['file_path']}, line {func['start_line']})") # lines {func['start_line']}-{func['end_line']})")
    

    c_code = get_unit_code(one_unit)
    c_path = f"{database_dir}/tmp.c"
    write_file(c_path, c_code)

    chunk_tokens = 40000
    lined_code = get_lined_code(c_path, database_dir, chunk_tokens=False)
    should_split, first_chunk, remaining_paths, c_codes = split_code_by_tokens(lined_code, work_dir, chunk_tokens)

    prompt.extend(["\n## Response format", "In summary, please respond in the following JSON format:"]) 
    prompt.extend([translate_template])

    if should_split is False:
        c_code = get_lined_code(c_path, database_dir, chunk_tokens)
        prompt.extend([
            f"\n## C code segment:", 
            c_code
        ])

    else:
        prompt.extend([
            f"\n## C code segment:", 
            f"The C code segment was too long, so it has been split into multiple parts.", 
            f"The first part is below.",
            f"The remaining parts are stored at paths {remaining_paths}, so please read those files one by one, in order using read_data mode before proceeding with the translation.",
            #f"After this, I will send the fragment information. Until I am done, do not respond with anything other than the JSON answer {{\"status\": \"ok\"}}.",
            "",
            first_chunk, 
        ])
        c_code = first_chunk

        """
        prompt_snapshot = prompt.copy()

        prompt.extend(["\n## Response format", 
                        f"After this, I will send the fragment information. Until I am done, do not respond with anything other than the JSON answer {{\"status\": \"ok\"}}."]) 
        #prompt.extend([translate_template])

        rsp_json = ask_llm(prompt, "init", llm_interface)

        for i in range(len(remaining_paths)):
            path = remaining_paths[i]
            c_code = c_codes[i]
            remaining_prompt = []
            remaining_prompt.extend([
            f"## C code segment of {path}:", 
                f"The subsequent part is below.",
                f"Until I am done, do not respond with anything other than the JSON answer {{\"status\": \"ok\"}}.",
                "",
                c_code, 
            ])
            rsp_json = ask_llm(remaining_prompt, "continue", llm_interface)

        prompt = prompt_snapshot.copy()

        prompt.extend(["\n## Response format", "In summary, please respond in the following JSON format:"]) 
        prompt.extend([translate_template])
        """

    # Display the previous read_data mode
    if read_prompt is not None:
        prompt.extend(["", "## Response to the previous request:"])
        prompt.extend(read_prompt)
        read_prompt = None # Initialization

    rust_code = get_lined_code(rust_path, work_dir)
    prompt.extend([f"\n## Existing code already in {rust_path}:",
    rust_code
    ])

    prompt.extend(["", "## Directory structure of the translated Rust program:"])  
    directory_structure = get_dir_struct("translation", work_dir, None)
    prompt.extend([directory_structure, ""])

    prompt.extend(["", "## Module structure of the Rust program:"])
    structure = get_cargo_modules(rust_output_dir)
    prompt.extend([structure, ""])

    
    global current_c_block_end
    current_c_block_end = 1 #0

    ref_files = []

    ongoing_flag = False
    no_omission = None
    rsp_json = {}
    current_block_complete = None
    refined_completed = None
    ongoing_count = 0
    read_prompt = None
    exp_data = {}

    init_rust_end = count_file_lines(rust_path) + 1

    while (1):

        if ongoing_flag:
            total_end = count_file_lines(c_path) 
            if current_c_block_end == total_end:
                break
            print("Keep going to receive Rust code.")
            
            #if no_omission is not False and current_block_complete is not False: # and refined_completed is not False:
            prompt = [f"Please translate the remaining code from after line {current_c_block_end} as shown below. Do NOT translate any code before line {current_c_block_end}.",
                    "Keep following the translation rules.",
                    "",
                    "## Translation rules:",
                    #"- All symbols referenced by the C code below (constants, types, macros, helper functions, global variables) are already defined in either bindings.rs (generated by build.rs) or the existing lib.rs. Use them directly by name and never redefine them.",
                    "- Translate all code in the C code segment below into safe Rust. All symbols that are referenced within the C code segment below but whose definition exists outside the segment (constants, macros, helper functions, global variables, type definitions defined in other segments) are already defined in either bindings.rs or the existing lib.rs. Use them directly by name and never redefine them.",
                    "- Distinguish between definitions and declarations in the C code segment:",
                    "    - Definitions (MUST translate): translate every construct in the C segment that carries content of its own — i.e. anything with a body or an initializer.",
                    #"    - Definitions (MUST translate): C code that has a body, including enum and struct/union definitions with their members, and conditional typedefs inside #ifdef/#ifndef blocks — not only functions.",
                    "    - Declarations (do NOT translate): function prototypes without body, forward declarations without braces (e.g. struct Foo;, typedef struct Foo Foo;), and extern declarations without initializer. Do NOT invent an implementation for it by yourself.",
                    "- Do NOT include any omissions or simplification in your answer code.", 
                    #"- Please do not overlap the c_block_start and c_block_end values with previous responses",
                    #"- Only perform the Rust translation within the range of the original C code initially presented. Do not add any new functions or extensions, and translate only the provided C code.", #"When the translation is complete, set 'ongoing': false.",
                    # "- Perform the translation corresponding to the initially presented C original code. Even if you know the entire program, do not add or extend code beyond the presented C source code.",
                    "- Please do not use unsafe, raw pointers, or manual memory management as much as possible.",
                    "    - EXCEPTION: Permit minimal unsafe blocks strictly limited to the following two categories.",
                    "            - Stub implementation of the FFI boundary functions (specified in ## FFI boundary functions below): These C functions are being replaced by Rust. These are declared as extern C fn with #[unsafe(no_mangle)] so that C code can call the Rust replacement. Stub implementation of the FFI boundary functions MUST remain unchanged until you are explicitly instructed to replace them with the actual implementation. Do NOT implement the actual logic of it.", # Note that even inside FFI boundary functions, extract logic into safe Rust helper functions.",
                    #"            - Global variables: C global variables shared across the boundary, accessed through unsafe extern C static declarations with getter and setter functions.",
                    "    - For everything else, write in safe Rust without calling C functions through FFI.",
                    #"- Do not use unsafe or raw pointers. Instead, please use safe Rust types and operations including custom structs, enums, Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.", 
                    #"- Avoid using unsafe, and use the Rust standard library or crates to achieve equivalent functionality in a safe manner.",
                    #"- When translating C raw pointers to Rust, use appropriate safe representations such as references, slices, Option, Box, Rc, Arc, etc.",
                    #"- Declare all items (structs, enums, functions, constants, etc.) with pub (public) so they can be imported from other modules.", # This also applies to methods.
                    "- Define all functions and types at the top level. Avoid using the mod keyword for module partitioning and avoid defining methods for types (within impl blocks).", #"- Define all functions and types at the top level without using the mod keyword.",
                    f"- The translated code content should be ready to run as-is, without omissions, and should include entire units such as functions or data types, starting from lines without indentation.", # Ensure the code is complete without omission and includes entire units like functions or data types that can be parsed by ctags.
                    "- When representing backslashes as byte literals, escape the backslash twice in the source code, and also escape it again in the byte literal, resulting in four backslashes (double backslashes).",
                    "- When representing backslashes as character literals, escape the backslash once in the source code and again in the character literal, resulting in two backslashes.",
                    "- After translating functions to Rust, please add the prefix \"rust_\" to all function names.",
                    # f"Continue providing the code in a format that can be parsed by json.loads(), within {output_max} tokens, and record it in the value of the 'rust_code' key in JSON format.", # Return the code in JSON format that can be parsed by json.loads()
                    # "Ensure that escape characters (e.g., newlines, double quotes, backslashes) are appropriately escaped in the JSON response.",
                    # f"Continue encoding the code in Base64, within {output_max} tokens, and write it in the value of the 'rust_code' key in JSON format.",
                    f"- Continue providing the code, keeping it within {output_max} tokens, and record it in the value of the 'rust_code' key in JSON format.",
                    "- If your answer is likely to exceed the token limit, please split the response into multiple parts.", # but it's omitted for brevity as it would exceed the token limit.
                    "- If the translated code is the last part, set the 'ongoing' key to a boolean value of False. If there is remaining code, set the 'ongoing' key to a boolean value of True.",
                    # "Ensure that the Rust code provided in one response does not interrupt function or struct definitions midway.",
                    #"- Instead of trying to strictly reproduce the exact details of the original code, aim to achieve similar functionality using idiomatic Rust approaches.",
                    #"- Ensure that the function inputs and outputs match between the original C code and the translated Rust code (maintain equivalence).",
                    # "Do not add a main function unless the original C code contains one.",
                    # f"I'm saying this again because it's important: Do not try to convert everything at once. Keep the code included in 'rust_code' within {output_max} tokens for one part. Please strictly follow this instruction.",
                    ]

            if len(target_function) != 0:
                prompt.extend([
                    "- For the following entry point functions within the FFI boundary functions, make them callable from C code as they will be called directly from C code as entry points.",
                    "- For the entry point functions, if a stub implementation already exists, delete the stub implementation first, then write the actual implementation as a complete replacement of it.",
                    "- If the entry point function is named 'main', its Rust function name should be made unique by appending _main to the filename (stripped of its extension, with non-alphanumeric characters replaced by _). If a collision still occurs, prefix the parent directory name.",
                    "- IMPORTANT: This Rust code is a LIBRARY, not a standalone binary. You MUST NEVER define a function named main or fn main() on the Rust side. The Rust 'main' entry-point functions are called from the C-side main functions as library functions.",
                ])

                prompt.extend(["  - Entry point functions:"])
                for func in target_function:
                    prompt.extend([f"    - {func}"])

            
            prompt.extend(["\n## FFI boundary functions:"])
            for func in functions:
                prompt.append(f"   - {func['name']} (from {func['file_path']}, line {func['start_line']})")
            
            prompt.extend(["\n## Response format", "In summary, please respond in the following JSON format:"]) 
            prompt.extend([translate_template])

            """
            c_code = get_lined_specific_code(database_dir, c_path, current_c_block_end, total_end)
            prompt.extend([
                f"\n## C source code:", # (lines {current_c_block_end} - {total_end} in the file {c_path}):",
                c_code
            ])
            """

        # Display the previous read_data mode
        if read_prompt is not None:
            prompt.extend(["", "## Response to the previous request:"])
            prompt.extend(read_prompt)
            read_prompt = None # Initialization
              
        exp_data['repair_count'] = 0
        exp_data['phase'] = 'convert'
        if ongoing_count == 0:

            if INSERT_FILES:
                if len(ref_files) != 0:
                    rust_ref_files = []
                    for ref in ref_files:
                        ref_rust_path = get_rust_path(ref, map_path)
                        ref_base_path = remove_base_path(ref_rust_path, rust_output_dir)
                        rust_ref_files.append(ref_base_path)

                    history_path = []
                    base_rust_path = remove_base_path(rust_path, rust_output_dir)
                    history_path.append(base_rust_path) #history_path.append(rust_path)
                    history_path.extend(rust_ref_files)
                    
                    rsp_json = ask_llm(prompt, history_path, llm_interface) 
                    if 'c_block_end' in rsp_json:
                        c_block_end = rsp_json['c_block_end']
                        current_c_block_end = c_block_end 
                        
                else:
                    rsp_json = ask_llm(prompt, "init", llm_interface)

            else:
                rsp_json = ask_llm(prompt, "init", llm_interface)

        else:
            if not W_O_DEP:
                prompt.extend(["", "## Module structure of the target Rust program:"])
                structure = get_cargo_modules(rust_output_dir)
                prompt.extend([structure, ""])
                

            rsp_json = ask_llm(prompt, "continue", llm_interface)
            if 'c_block_end' in rsp_json:
                c_block_end = rsp_json['c_block_end']
                current_c_block_end = c_block_end 

        if 'c_block_end' in rsp_json:
            c_block_end = rsp_json['c_block_end']
            current_c_block_end = c_block_end 

        toml_submit = False
        build_submit = False

        if 'ongoing' in rsp_json:
            ongoing_flag = rsp_json['ongoing']
            #ongoing_flag = search_key('ongoing', rsp_json)
        else:
            print("Should include ongoing flag") 
        
        if 'mode' in rsp_json:
            mode = rsp_json['mode']

            if mode == "write_rust":
                if 'rust_code' in rsp_json:
                    append_rust_path(rust_path, rsp_json['rust_code'])
                
                if 'toml' in rsp_json: #if len(rsp_json) > 1:
                    if rsp_json['toml'] is not None:
                        # Here, rewrite the entire Cargo.toml
                        cargo_toml_path = rust_output_dir + "/" + "Cargo.toml"
                        #write_toml(rsp_json['toml'], cargo_toml_path) # Write TOML data to file

                        # Instead of rewriting the entire Cargo.toml, merge into it
                        existing_data = load_toml_file(cargo_toml_path)
                        merge_toml_json(existing_data, rsp_json['toml']) # Overwrite existing data with new JSON data
                        updated_toml_data = toml.dumps(existing_data)  # Convert the overwritten data to TOML format
                        write_toml_file(updated_toml_data, cargo_toml_path) # Write TOML data to file
                
                if 'no_omission' in rsp_json:
                    no_omission = rsp_json['no_omission']
                    if no_omission is False:
                        ongoing_flag = True

                if 'current_block_complete' in rsp_json:
                    current_block_complete = rsp_json['current_block_complete']
                    if current_block_complete is False:
                        ongoing_flag = True
            
            elif mode == "read_data":
                read_prompt = []
                sum_target_list = []
                sum_slice_list = []

                if 'target_files' in rsp_json and rsp_json['target_files'] is not None:
                    target_list = rsp_json['target_files']
                    if not isinstance(target_list, list):
                        target_list = [target_list]
                    sum_target_list.extend(target_list)

                if 'file_slices' in rsp_json and rsp_json['file_slices'] is not None:
                    slice_list = rsp_json['file_slices']
                    if not isinstance(slice_list, list):
                        slice_list = [slice_list]
                    sum_slice_list.extend(slice_list)

                read_prompt = ["The content obtained in read_data mode is as follows.", ""]
            
                for see_path in sum_target_list:
                    file_code = get_lined_code(see_path, work_dir)
                    read_prompt.extend([f"- Content of the file {see_path}:"])  
                    if len(file_code) == 0:
                        file_code = f"Line 1 [0]: [This {see_path} file is currently empty and contains no content. *** STOP *** Do not use read_data mode anymore.]"
                    read_prompt.extend([f'{file_code}\n'])

                for see_item in sum_slice_list:
                    file_code = get_lined_specific_code(database_dir, see_item['file_path'], see_item['start_line'], see_item['end_line'])
                    read_prompt.extend([f"- Content of {see_item['start_line']} - {see_item['end_line']} lines in the file {see_item['file_path']}:"]) 
                    read_prompt.extend([f'{file_code}\n'])
                    
            
        iteration_dict[rust_path] = 1

        if not ongoing_flag:
            break

        ongoing_count += 1


    last_rust_end = count_file_lines(rust_path)

    copy_file(rust_path, f"{database_dir}/unrefined.rs")
    refined_rust_code = read_specific_lines(rust_path, 1, init_rust_end-1)
    #refined_rust_code = get_lined_specific_code(rust_path, 1, init_rust_end-1)
    delete_file(rust_path)
    write_file(rust_path, refined_rust_code)

    prompt = []
    prompt.extend([f"The following Rust code (lines {init_rust_end}-{last_rust_end} in {rust_path}) was generated from the original C code.",
                   "Please refine this code to be more idiomatic Rust, focusing on safety and aligning it with Rust's principles rather than making it a direct translation from C.",
                   #"Please start writing the complete code from c_block_start = 1, even if no changes are made to the code."
                   ])

    prompt.extend(["", "## Response rules",  
                    #"- All symbols referenced by the C code below (constants, types, macros, helper functions, global variables) are already defined in either bindings.rs (generated by build.rs) or the existing lib.rs. Use them directly by name and never redefine them.",
                    "- Translate all code in the C code segment below into safe Rust. All symbols that are referenced within the C code segment below but whose definition exists outside the segment (constants, macros, helper functions, global variables, type definitions defined in other segments) are already defined in either bindings.rs or the existing lib.rs. Use them directly by name and never redefine them.",
                    "- Distinguish between definitions and declarations in the C code segment:",
                    "    - Definitions (MUST translate): translate every construct in the C segment that carries content of its own — i.e. anything with a body or an initializer.",
                    #"    - Definitions (MUST translate): C code that has a body, including enum and struct/union definitions with their members, and conditional typedefs inside #ifdef/#ifndef blocks — not only functions.",
                    "    - Declarations (do NOT translate): function prototypes without body, forward declarations without braces (e.g. struct Foo;, typedef struct Foo Foo;), and extern declarations without initializer. Do NOT invent an implementation for it by yourself.",
                    #"- In particular, do NOT use unsafe or raw pointers. Instead, please use appropriate safe alternatives like Box, Arc, Vec, and others.",
                    #"- Do not use unsafe or raw pointers. Instead, please use safe Rust types and operations including custom structs, enums, Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.", #"- NEVER use unsafe or raw pointers. Only use safe Rust types and operations: Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.",
                    "- Please do not use unsafe, raw pointers, or manual memory management as much as possible.",
                    "    - EXCEPTION: Permit minimal unsafe blocks strictly limited to the following two categories.",
                    "              - Stub implementation of the FFI boundary functions (specified in ## FFI boundary functions below): These C functions are being replaced by Rust. These are declared as extern C fn with #[unsafe(no_mangle)] so that C code can call the Rust replacement. Stub implementation of the FFI boundary functions MUST remain unchanged until you are explicitly instructed to replace them with the actual implementation. Do NOT implement the actual logic of it.", # Note that even inside FFI boundary functions, extract logic into safe Rust helper functions.",
                    #"              - Global variables: C global variables shared across the boundary, accessed through unsafe extern C static declarations with getter and setter functions.",
                    "    - For everything else, write in safe Rust without calling C functions through FFI.",
                    #"- For global variables, prefer const for compile-time constants. For runtime mutable global variables, do NOT use static mut - instead, choose an appropriate type based on the purpose (atomic types, Cell types, OnceCell/Lazy, Mutex/RwLock, etc). Consider whether global state is truly necessary, and if it is, try grouping related variables into a struct.", #"- For global variables, do NOT use static mut. Instead, choose an appropriate type based on the purpose of use. First, consider whether global state is truly necessary - prefer local variables when possible. If global state is necessary, consider grouping related variables into a struct where possible, then select an appropriate type based on usage from options such as atomic types, const, Cell types, OnceCell/Lazy, Mutex/RwLock, etc.", #"- For global variables, do NOT use static mut. Instead, choose an appropriate type based on the purpose of use and whether it will be accessed across threads, such as const, Cell types, OnceCell/Lazy, atomic types, or Mutex/RwLock.",
                    "- Maintain complete functional equivalence with the original C implementation while leveraging Rust's safety features. Do not omit or simplify any functionality from the original code.",
                    "- It is important to note that you should NOT simplify the code for safety. Do NOT include any omissions or simplification in your answer code.", 
                    #"- However, the primary goal is to fully implement the original C functionality without simplification. If the implementation cannot be achieved without using unsafe or other C-based constructs, their use is permitted.",
                    #"- Declare all items (structs, enums, functions, constants, etc.) with pub (public) so they can be imported from other modules.",
                    "- Define all functions and types at the top level. Avoid using the mod keyword for module partitioning and avoid defining methods for types (within impl blocks).", #
                    # "- When writing functions, first add 'use tracing::info;' at the top of the file, then add log outputs using the info!() macro in each function as follows so that the values of the arguments and the return values can be traced:",
                    # "    - Just after function entry, log the argument values with: \"Enter_log::: variable_name1 = value1, variable_name2 = value2, ...\", and just before function exit, log the return values with: \"Return_log::: variable_name1 = value1, variable_name2 = value2, ...\"",
                    # "    - If an argument or return value is a pointer, please log its value rather than its address.",
                    # "    - If there are no arguments or return values, you do not need to include an info!() log output.",
                    # "    - For any argument or return value that does not implement the Debug trait (and thus cannot be used with the tracing crate), log its value as <unprintable>.",
                    # "- To ensure that the tracing crate can be used, please implement the Debug trait for all data types defined in the code that are used as function argument or return types.",
                    #"- For strings that are treated as literal strings in the original C code, ensure that in the translated Rust code these strings are identical for equality checks -this includes preserving case sensitivity (uppercase/lowercase) and making sure every single character matches perfectly. Similarly, any strings originally printed via printf should be translated into Rust exactly as they appear in the original C code.",             
                    f"- To avoid hitting the token limit, keep the JSON data included in one response within {output_max} tokens, and provide only the first part of the JSON data with clear separation for now, even if the response will be split into multiple parts.", # If it's a long response,
                    f"- If the response is split into multiple parts and there is still remaining JSON data, write a boolean value of True for the 'ongoing' key. If the JSON data is the final part, write a boolean value of False for the 'ongoing' key.",
                   ])
    
    # get_context_prompt
    prompt.extend(sole_prompt)

    prompt.extend(["\n## FFI boundary functions:"])
    for func in functions:
        prompt.append(f"   - {func['name']} (from {func['file_path']}, line {func['start_line']})") # lines {func['start_line']}-{func['end_line']})")
    

    rust_code = get_lined_specific_code(database_dir, f"{database_dir}/unrefined.rs", init_rust_end, last_rust_end)
    prompt.extend([
        f"\n## Rust source code ({rust_path}):",
        rust_code
    ])

    lined_code = get_lined_code(c_path, database_dir, chunk_tokens=False)
    should_split, first_chunk, remaining_paths, c_codes = split_code_by_tokens(lined_code, work_dir, chunk_tokens)

    prompt.extend(["\n## Response format", "Please provide your response in the following JSON format:"]) 
    prompt.extend([refine_template])

    if should_split is False:
        prompt.extend([
            f"\n## C code segment:", 
            c_code
        ])

    else:
        prompt.extend([
            f"\n## C code segment:", 
            f"The C code segment was too long, so it has been split into multiple parts.", 
            f"The first part is below.",
            f"The remaining parts are stored at paths {remaining_paths}, so please read those files one by one, in order using read_data mode before proceeding with the translation.",
            #f"After this, I will send the fragment information. Until I am done, do not respond with anything other than the JSON answer {{\"status\": \"ok\"}}.",
            "",
            first_chunk, 
        ])
        c_code = first_chunk

        """
        prompt_snapshot = prompt.copy()

        prompt.extend(["\n## Response format", 
                        f"After this, I will send the fragment information. Until I am done, do not respond with anything other than the JSON answer {{\"status\": \"ok\"}}."]) 
        #prompt.extend([translate_template])

        rsp_json = ask_llm(prompt, "init", llm_interface)

        for i in range(len(remaining_paths)):
            path = remaining_paths[i]
            c_code = c_codes[i]
            remaining_prompt = []
            remaining_prompt.extend([
            f"## C code segment of {path}:", 
                f"The subsequent part is below.",
                f"Until I am done, do not respond with anything other than the JSON answer {{\"status\": \"ok\"}}.",
                "",
                c_code, 
            ])
            rsp_json = ask_llm(remaining_prompt, "continue", llm_interface)

        prompt = prompt_snapshot.copy()

        prompt.extend(["\n## Response format", "Please provide your response in the following JSON format:"]) 
        prompt.extend([refine_template])
        """

    # Display the previous read_data mode
    if read_prompt is not None:
        prompt.extend(["", "## Response to the previous request:"])
        prompt.extend(read_prompt)
        read_prompt = None # Initialization


    prompt.extend(["", "## Directory structure of the translated Rust program:"])  
    directory_structure = get_dir_struct("translation", work_dir, None)
    prompt.extend([directory_structure, ""])

    prompt.extend(["", "## Module structure of the Rust program:"])
    structure = get_cargo_modules(rust_output_dir)
    prompt.extend([structure, ""])

    save_prompt = prompt

    ref_files = []
    ongoing_flag = False
    rust_block_start = None
    rust_block_end = None
    current_c_block_end = 1 #0

    while (1):
        if ongoing_flag:
            #if no_omission is not False and current_block_complete is not False:
            print("Keep going to receive Rust code.")

            prompt = [#"Please continue writing the translated code.",
                    f"Please provide only the refined code corresponding to current code lines {current_c_block_end} onwards. Do NOT include refined code for current code lines before {current_c_block_end}.",
                    "Keep following the rules below.",
                    "",
                    "## Response rules",
                    #"- All symbols referenced by the C code below (constants, types, macros, helper functions, global variables) are already defined in either bindings.rs (generated by build.rs) or the existing lib.rs. Use them directly by name and never redefine them.",
                    "- Translate all code in the C code segment below into safe Rust. All symbols that are referenced within the C code segment below but whose definition exists outside the segment (constants, macros, helper functions, global variables, type definitions defined in other segments) are already defined in either bindings.rs or the existing lib.rs. Use them directly by name and never redefine them.",
                    "- Distinguish between definitions and declarations in the C code segment:",
                    "    - Definitions (MUST translate): translate every construct in the C segment that carries content of its own — i.e. anything with a body or an initializer.",
                    #"    - Definitions (MUST translate): C code that has a body, including enum and struct/union definitions with their members, and conditional typedefs inside #ifdef/#ifndef blocks — not only functions.",
                    "    - Declarations (do NOT translate): function prototypes without body, forward declarations without braces (e.g. struct Foo;, typedef struct Foo Foo;), and extern declarations without initializer. Do NOT invent an implementation for it by yourself.",
                    #"- Please do not overlap the c_block_start and c_block_end values with previous responses",
                    #"- Only perform the Rust translation within the range of the original C code initially presented. Do not add any new functions or extensions, and translate only the provided C code.", #"When the translation is complete, set 'ongoing': false.",
                    #"- Do not use unsafe or raw pointers. Instead, please use safe Rust types and operations including custom structs, enums, Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.", #"- NEVER use unsafe or raw pointers. Only use safe Rust types and operations: Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.",
                    "- Please do not use unsafe, raw pointers, or manual memory management as much as possible.",
                    "    - EXCEPTION: Permit minimal unsafe blocks strictly limited to the following two categories.",
                    "          - Stub implementation of the FFI boundary functions (specified in ## FFI boundary functions below): These C functions are being replaced by Rust. These are declared as extern C fn with #[unsafe(no_mangle)] so that C code can call the Rust replacement. Stub implementation of the FFI boundary functions MUST remain unchanged until you are explicitly instructed to replace them with the actual implementation. Do NOT implement the actual logic of it.", # Note that even inside FFI boundary functions, extract logic into safe Rust helper functions.",
                    #"          - Global variables: C global variables shared across the boundary, accessed through unsafe extern C static declarations with getter and setter functions.",
                    "    - For everything else, write in safe Rust without calling C functions through FFI.",
                    #"- Avoid using unsafe, and use the Rust standard library or crates to achieve equivalent functionality in a safe manner.",
                    #"- When translating C raw pointers to Rust, use appropriate safe representations such as references, slices, Option, Box, Rc, Arc, etc.",
                    #"- Declare all items (structs, enums, functions, constants, etc.) with pub (public) so they can be imported from other modules.", # This also applies to methods.
                    "- Define all functions and types at the top level. Avoid using the mod keyword for module partitioning and avoid defining methods for types (within impl blocks).", #"- Define all functions and types at the top level without using the mod keyword.",
                    # "- Perform the translation corresponding to the initially presented C original code. Even if you know the entire program, do not add or extend code beyond the presented C source code.",
                    "- If your answer is likely to exceed the token limit, please split the response into multiple parts.", # but it's omitted for brevity as it would exceed the token limit.
                    "- When representing backslashes as byte literals, escape the backslash twice in the source code, and also escape it again in the byte literal, resulting in four backslashes (double backslashes).",
                    "- When representing backslashes as character literals, escape the backslash once in the source code and again in the character literal, resulting in two backslashes.",
                    # f"Continue providing the code in a format that can be parsed by json.loads(), within {output_max} tokens, and record it in the value of the 'rust_code' key in JSON format.", # Return the code in JSON format that can be parsed by json.loads()
                    # "Ensure that escape characters (e.g., newlines, double quotes, backslashes) are appropriately escaped in the JSON response.",
                    # f"Continue encoding the code in Base64, within {output_max} tokens, and write it in the value of the 'rust_code' key in JSON format.",
                    f"- Continue providing the code, keeping it within {output_max} tokens, and record it in the value of the 'rust_code' key in JSON format.",
                    f"- The translated code content should be ready to run as-is, without omissions, and should include entire units such as functions or data types, starting from lines without indentation.", # Ensure the code is complete without omission and includes entire units like functions or data types that can be parsed by ctags.
                    "- If the translated code is the last part, set the 'ongoing' key to a boolean value of False. If there is remaining code, set the 'ongoing' key to a boolean value of True.",
                    # "Ensure that the Rust code provided in one response does not interrupt function or struct definitions midway.",
                    #"- Instead of trying to strictly reproduce the exact details of the original code, aim to achieve similar functionality using idiomatic Rust approaches.",
                    #"- Ensure that the function inputs and outputs match between the original C code and the translated Rust code (maintain equivalence).",
                    # "Do not add a main function unless the original C code contains one.",
                    # f"I'm saying this again because it's important: Do not try to convert everything at once. Keep the code included in 'rust_code' within {output_max} tokens for one part. Please strictly follow this instruction.",
                    ]

            prompt.extend(["\n## FFI boundary functions:"])
            for func in functions:
                prompt.append(f"   - {func['name']} (from {func['file_path']}, line {func['start_line']})") # lines {func['start_line']}-{func['end_line']})")
            

            prompt.extend(["\nPlease provide your response in the following JSON format:"])
            prompt.extend([refine_template])

            """
            else:
                lack = ""
                if no_omission is False and current_block_complete is False:
                    lack = "omitted parts and code that does not implement the same functionalities as the original C code"
                elif no_omission is False:
                    lack = "omitted parts"
                elif current_block_complete is False:
                    lack = "code that does not fully implement the same functionalities as the original C code"

                prompt = [f"The previously provided code contains {lack}."]
                prompt.extend([f"Please follow the instructions and rewrite the Rust code without including any {lack}.", ""])

                prompt.extend(["- Please provide your response in the following JSON format:"])
                prompt.extend([refine_template])

                c_block_start = None
                c_block_end = None

                if 'c_block_start' in rsp_json:
                    c_block_start = rsp_json['c_block_start']
                
                if 'c_block_end' in rsp_json:
                    c_block_end = rsp_json['c_block_end']

                
                if c_block_start is not None and c_block_end is not None:
                    c_code = get_lined_specific_code(c_path, c_block_start, c_block_end)
                    prompt.extend([
                        f"\n## C source code ({c_path})（{c_block_start} - {c_block_end} lines）:",
                        c_code
                    ])
                else:
                    c_code = get_lined_code(c_path, work_dir)
                    prompt.extend([
                        f"\n## C source code ({c_path}):",
                        c_code
                    ])


                # c_code = get_lined_code(c_path, work_dir)
                # prompt.extend([
                #     f"\n## C source code ({c_path}):",
                #     c_code
                # ])

                rust_code = get_lined_code(rust_path, work_dir)
                prompt.extend([
                    f"\n## Previously provided Rust code ({rust_path}):", 
                    rust_code
                ])

                prompt.extend(["", "## Directory structure of the translated Rust program:"])  
                directory_structure = get_dir_struct(work_dir)  #rust_output_dir)
                prompt.extend([directory_structure, ""])

                prompt.extend(["", "## Module structure of the Rust program:"])
                structure = get_cargo_modules(rust_output_dir)
                prompt.extend([structure, ""])

                delete_file(rust_path)
                create_file(rust_path)

                #prompt.extend(save_prompt)
            """
            
            exp_data['repair_count'] = 0
            exp_data['phase'] = 'convert'
            
        if rust_block_start is not None and rust_block_end is not None:
            #rust_code = get_lined_specific_code("unrefined.rs", rust_block_start, rust_block_end)
            #rust_code = get_lined_code(f"{database_dir}/unrefined.rs", work_dir)
            rust_code = read_file(f"{database_dir}/unrefined.rs")
            prompt.extend([
                #f"\n## Unrefined Rust code（{rust_block_start} - {rust_block_end} lines）:",
                f"\n## Unrefined Rust code:",
                rust_code
            ])
            
        rsp_json = ask_llm(prompt, "continue", llm_interface)
        
        if 'c_block_end' in rsp_json:
            c_block_end = rsp_json['c_block_end']
            current_c_block_end = c_block_end 

        if 'rust_block_start' in rsp_json:
            rust_block_start = rsp_json['rust_block_start']
        
        if 'rust_block_end' in rsp_json:
            rust_block_end = rsp_json['rust_block_end']
            current_c_block_end = rust_block_end 

        toml_submit = False
        build_submit = False

        if 'ongoing' in rsp_json:
            ongoing_flag = rsp_json['ongoing']
            #ongoing_flag = search_key('ongoing', rsp_json)
        else:
            print("Should include ongoing flag") 
        
        if 'mode' in rsp_json:
            mode = rsp_json['mode']

            if mode == "write_rust":
                mode = rsp_json['mode']

                if 'rust_code' in rsp_json: #len(rsp_blocks) > 0:
                    append_rust_path(rust_path, rsp_json['rust_code']) #toml_submit, build_submit = update_rust_path(rust_path, rsp_json, rust_output_dir)
                
                if 'toml' in rsp_json: #if len(rsp_json) > 1:
                    if rsp_json['toml'] is not None:
                        # Here, rewrite the entire Cargo.toml
                        cargo_toml_path = rust_output_dir + "/" + "Cargo.toml"
                        #write_toml(rsp_json['toml'], cargo_toml_path) # Write TOML data to file

                        # Instead of rewriting the entire Cargo.toml, merge into it
                        existing_data = load_toml_file(cargo_toml_path)
                        merge_toml_json(existing_data, rsp_json['toml']) # Overwrite existing data with new JSON data
                        updated_toml_data = toml.dumps(existing_data) # Convert the overwritten data to TOML format
                        write_toml_file(updated_toml_data, cargo_toml_path) # Write TOML data to file
                
                if 'no_omission' in rsp_json:
                    no_omission = rsp_json['no_omission']
                    if no_omission is False:
                        ongoing_flag = True

                if 'current_block_complete' in rsp_json:
                    current_block_complete = rsp_json['current_block_complete']
                    if current_block_complete is False:
                        ongoing_flag = True

                if 'refined_completed' in rsp_json:
                    refined_completed = True

            elif mode == "read_data":
                read_prompt = []
                sum_target_list = []
                sum_slice_list = []

                if 'target_files' in rsp_json and rsp_json['target_files'] is not None:
                    target_list = rsp_json['target_files']
                    if not isinstance(target_list, list):
                        target_list = [target_list]
                    sum_target_list.extend(target_list)

                if 'file_slices' in rsp_json and rsp_json['file_slices'] is not None:
                    slice_list = rsp_json['file_slices']
                    if not isinstance(slice_list, list):
                        slice_list = [slice_list]
                    sum_slice_list.extend(slice_list)

                read_prompt = ["The content obtained in read_data mode is as follows.", ""]
            
                for see_path in sum_target_list:
                    file_code = get_lined_code(see_path, work_dir)
                    read_prompt.extend([f"- Content of the file {see_path}:"])  
                    if len(file_code) == 0:
                        file_code = f"Line 1 [0]: [This {see_path} file is currently empty and contains no content. *** STOP *** Do not use read_data mode anymore.]"
                    read_prompt.extend([f'{file_code}\n'])

                for see_item in sum_slice_list:
                    file_code = get_lined_specific_code(database_dir, see_item['file_path'], see_item['start_line'], see_item['end_line'])
                    read_prompt.extend([f"- Content of {see_item['start_line']} - {see_item['end_line']} lines in the file {see_item['file_path']}:"]) 
                    read_prompt.extend([f'{file_code}\n'])
                    

        iteration_dict[rust_path] = 1

        # if refined_completed is True:
        #     break
        if not ongoing_flag:
            break

        ongoing_count += 1


def repair_total(convert_element, prompt, c_path, rust_path, exp_data, error): # , start_line, end_line
    
    ref_files = []
    div_found = False
    dep_data = read_json(dep_json_path)
    for item in dep_data:
        parent_c_path = item['source']
        if item['source'] == c_path:
            ref_files = item['include'] + item['indirect_include']
        else:
            if 'div_parts' in item:
                parts = item['div_parts']
                for part in parts:
                    if part['source'] == c_path:
                        div_start_line = part['start_line']
                        div_end_line = part['end_line']
                        div_found = True
                        if 'include' in part:
                            ref_files = ref_files + part['include']
        if div_found:
            break
    
    ongoing_flag = False
    first_touch = True

    ongoing_count = 0

    while (1):
        exp_data['structure_repair'] = True
        if "repair_count" in item:
            del item["repair_count"]

        if ongoing_flag:
            print("Keep going to receive Rust code.")        
            prompt = ["Please continue writing the corrected code.",
                      f"Additionally, please ensure the answer's code is within {output_max} tokens and return it in JSON format under the key 'rust_code', which can be parsed by json.loads() into a valid JSON.",
                       "The corrected code should not include any omitted parts.", # "Please write the entire code that can be copied and compiled as is.",  # "Please write the entire unit such as functions or data types that can be parsed by ctags, starting from lines without indentation.", 
                       "If this is the last part of the corrected code, set the value of the 'ongoing' key to False. Furthermore, if there is more code remaining, set the value of the 'ongoing' key to True.",
                       # "For external function calls in the corrected code, avoid using unsafe and try to achieve the same functionality in a safe way using Rust's standard library or crates as much as possible.", # added A
                       # "This is important, so I'll say it again: do not try to answer everything at once. Please include only one part of the code within {output_max} tokens under 'rust_code'. This instruction must be followed.",
                       ]

        if ongoing_count == 0:
            prompt = []
            prompt.extend([f"The following Rust code (lines {init_rust_end}-{last_rust_end} in {rust_path}) was translated from the original C code, but the structure has become broken.",
                           #"Please refer to the original C source code and correct it into Rust code with a proper syntactic structure.",
                           "Please refer to the following compilation errors and repair the original Rust code so that the syntax structure is valid.",
                           f"Write the entire corrected code without omitting any parts, ensuring it can be copied and compiled as-is.", # "Please write the entire unit, such as functions or data types, starting from lines without indentation that can be parsed by ctags.",
                           # "For external function calls in the corrected code, avoid using unsafe and, if possible, use Rust's standard library or crates to achieve the same functionality safely.", # added A
                           "Provide the corrected code in JSON format under the 'rust_code' key, which can be parsed using json.loads().",
                           f"It is crucial that you adhere to the {output_max} token limit for each 'rust_code' content part. Please ensure this limit is strictly observed.",
                           f"When answering in multiple parts, if there is more code remaining, set the value of the 'ongoing' key to True. If this is the last part of the corrected code, set the 'ongoing' key to False.",
                           ])
            
            rust_code = read_file(rust_path)
            if not div_found:
                c_code = read_file(c_path)
            else:
                c_code = read_specific_lines(parent_c_path, div_start_line, div_end_line)

            prompt.extend(["## Current Rust code:", rust_code])

            prompt.extend(["", "## Error:", error])

            # Remove None
            prompt = [item for item in prompt if item is not None]
            rsp_json = ask_llm(prompt, "init", interface)
        else:
            rsp_json = ask_llm(prompt, "continue", interface) 
            if 'c_block_end' in rsp_json:
                c_block_end = rsp_json['c_block_end']
                current_c_block_end = c_block_end 


        if 'rust_code' in rsp_json: #len(rsp_blocks) > 0:
            if ongoing_count == 0:
                delete_file(rust_path)
            append_rust_path(rust_path, rsp_json['rust_code']) #toml_submit, build_submit = update_rust_path(rust_path, rsp_json, rust_output_dir)
        

        if 'toml' in rsp_json: #if len(rsp_json) > 1:
            if rsp_json['toml'] is not None:
                # Here, rewrite the entire Cargo.toml
                cargo_toml_path = rust_output_dir + "/" + "Cargo.toml"

                existing_data = load_toml_file(cargo_toml_path)
                merge_toml_json(existing_data, rsp_json['toml']) # Overwrite existing data with new JSON data
                updated_toml_data = toml.dumps(existing_data) # Convert the overwritten data to TOML format
                write_toml_file(updated_toml_data, cargo_toml_path) # Write TOML data to file

        
        
        if 'ongoing' in rsp_json:
            ongoing_flag = rsp_json['ongoing']
            #ongoing_flag = search_key('ongoing', rsp_json)
        else:
            print("Should include ongoing flag") 
        
        iteration_dict[rust_path] = 1

        if not ongoing_flag:
            break

        ongoing_count += 1


def rust_compile(target_directory):
    print("locally rust_compile")
    src_path = target_directory + "/Cargo.toml"
    command = [f"RUSTFLAGS=\"-Awarnings\" cargo build --release --manifest-path={src_path}"] # RUSTFLAGS="-Awarnings" cargo build --release --manifest-path=modified_rust/Cargo.toml
    # RUSTFLAGS="-Awarnings" cargo build --release --manifest-path=modified_rust/Cargo.toml
    # RUSTFLAGS="-Awarnings --cfg feature=\"UINTPTR_MAX_defined\"" cargo build --release --manifest-path=modified_rust/Cargo.toml
    output, error = execute_command(command)
    print(f"Executed for {src_path}: {command}")
    print("Captured Outputs:")
    print("Output:", output)
    print("Error:", error)
    
    return error


def create_rust_base_json(c_path, meta_dir, label, div_start_line):
    c_data = obtain_metadata(c_path, meta_dir, False, False, "def")

    # It might be better to present this block by block.
    # Because there are conditional blocks and such

    if div_start_line is None:
        div_start_line = 1
    else:
        div_start_line = 0

    new_data = []

    for item in c_data:
        if item['block_type'] != label:
            continue

        if 'end_line' in item:
            new_json = {
                "block_type": item['block_type'],
                "name": item['name'],
                "start_line": item['start_line'] - div_start_line + 1,
                "end_line": item['end_line'] - div_start_line + 1, # Some items do not have end_line
                "rust_code": None, # This part should be filled in by the LLM
            }
            new_data.append(new_json)
        else:
            new_json = {
                "block_type": item['block_type'],
                "name": item['name'],
                "start_line": item['start_line'] - div_start_line + 1,
                #"end_line": item['end_line'], # Some items do not have end_line
                "rust_code": None, # This part should be filled in by the LLM
            }
            new_data.append(new_json)

        if label != 'function':
            continue

        if 'components' in item:
            new_comps = []
            for elem in item['components']:
                if elem['category'] != 'function':
                    continue

                new_json = {
                    #"category": elem['category'],
                    "block_type": 'function',
                    "name": elem['name'],
                    "start_line": elem['start_line'] - div_start_line + 1,
                    "end_line": elem['end_line'] - div_start_line + 1, # Some items do not have end_line
                    "rust_code": None,
                }
                new_data.append(new_json) # The structure is transformed here into a flat connection

    tmp_rust_path = "rust_tmp.json"
    write_json(tmp_rust_path, new_data) # Write to rust_path here

    return tmp_rust_path


def insert_rust_def(file_path, element, rust_code):
    meta_data, meta_path = obtain_metadata(file_path, meta_dir, False, None, "def")
    for item in meta_data:
        if item['name'] == element:
            item['rust_code']['content'] = rust_code
            item['rust_macro_define'] = rust_code
        
        if 'components' in item:
            for com in item['components']:
                if com['name'] == element:
                    com['rust_code']['content'] = rust_code
                    com['rust_macro_define'] = rust_code

    write_json(meta_path, meta_data)


def insert_rust_macro_def(macro_path, all_macro_path, element, rust_code):
    #print("Inserting into all_macro_path")

    macro_json = read_json(macro_path)

    found = False
    if macro_json is not None:
        for macro_name, entries in macro_json.items():
            if macro_name == element:
                found = True
                for entry in entries:
                    entry['rust_macro_define'] = rust_code
                
                break
        if found:
            write_json(macro_path, macro_json)


    macro_json = read_json(all_macro_path)
    
    found = False
    if macro_json is not None:
        for macro_name, entries in macro_json.items():
            if macro_name == element:
                found = True
                for entry in entries:
                    entry['rust_macro_define'] = rust_code
        
        if found:
            write_json(all_macro_path, macro_json)

    """
    meta_data, meta_path = obtain_metadata(file_path, meta_dir, False, None, "def")
    for item in meta_data:
        if item['name'] == element:
            item['rust_code'] = rust_code
        
        if 'components' in item:
            for com in item['components']:
                if com['name'] == element:
                    com['rust_code'] = rust_code

    write_json(meta_path, meta_data)
    """

def get_start_from_persable_id(tmp_item):
    global persable_units

    start_line = None
    end_line = None

    for file_path, unit_items in persable_units.items():
        for unit_item in unit_items:
            if tmp_item['persable_id'] == unit_item['persable_id']:
                start_line = unit_item['start_line']
                end_line = unit_item['end_line']
                break
        
    return start_line

def merge_conds_rust_metadata(c_path, tmp_rust_path):
    print("In merge_conds_rust_metadata")
    tmp_data = read_json(tmp_rust_path)
    
    print("Read answer.json")

    c_meta_data, c_meta_path = obtain_metadata(c_path, meta_dir, False, None, "def")
    print(f"Merging to {c_meta_path} in merge_conds_rust_metadata")

    # Convert tmp_data into a dictionary using name as the key
    tmp_data_dict = {tmp_item['name']: tmp_item for tmp_item in tmp_data}

    new_meta_data = []
    
    for c_item in c_meta_data:
        print(f"For {c_item['name']}")
        
        # Process the main item
        if c_item['name'] in tmp_data_dict:
            tmp_item = tmp_data_dict[c_item['name']]
            c_item['rust_code']['content'] = tmp_item['rust_code']

        # Process components
        if 'components' in c_item:
            for com in c_item['components']:
                if com['name'] in tmp_data_dict:
                    tmp_item = tmp_data_dict[com['name']]
                    #com['rust_code'] = tmp_item['rust_code']
                    com['rust_code']['content'] = tmp_item['rust_code']

    # Add elements in tmp_data that are not in c_meta_data to new_meta_data
    existing_names = {c_item['name'] for c_item in c_meta_data}
    for tmp_item in tmp_data:
        if tmp_item['name'] not in existing_names:
            new_meta_data.append({
                "name": tmp_item['name'],
                "file_path": c_path,
                "start_line": None,
                "rust_code": {
                    "file_path" : None,
                    "start_line" : None,
                    "content" : tmp_item['rust_code']
                }
            })

    # Merge the updated data
    c_meta_data.extend(new_meta_data)

    write_json(c_meta_path, c_meta_data)


def refer_block_types(c_path, meta_dir):
    conds_flag = others_flag = funcs_flag = False
    meta_data = obtain_metadata(c_path, meta_dir, False, False, "def")
    for item in meta_data:
        if item['block_type'] == "conditional":
            conds_flag = True
        elif item['block_type'] == "others":
            others_flag = True
        elif item['block_type'] == "function":
            funcs_flag = True

    print(f"{conds_flag}, {others_flag}, {funcs_flag}")

    process_list = []
    if conds_flag:
        process_list.append('conditional')
    if others_flag: # this should be if, not elif
        process_list.append('others')
    if funcs_flag: # this should be if, not elif
        process_list.append('function')

    return process_list



def refer_parse_types(c_path, meta_dir):
    conds_flag = others_flag = funcs_flag = False
    meta_data, meta_path = obtain_metadata(c_path, meta_dir, False, None, "def")

    if meta_data is None:
        return []
    
    for item in meta_data:
        if item['block_type'] == "conditional":
            conds_flag = True
        elif item['block_type'] == "others":
            others_flag = True
        elif item['block_type'] == "function":
            funcs_flag = True

        if 'components' in item:
            for com in item['components']:
                if 'block_type' in com and com['block_type'] == "conditional":
                    conds_flag = True
                if com['category'] in ['macro_func', 'macro_var', 'data_type', 'global_var']:
                    others_flag = True
                if com['category'] == 'function':
                    funcs_flag = True

    print(f"meta_path is {meta_path}")
    print(f"{conds_flag}, {others_flag}, {funcs_flag}")

    process_list = []
    if conds_flag:
        process_list.append('conditional')
    if others_flag: # this should be if, not elif
        process_list.append('others')
    if funcs_flag: # this should be if, not elif
        process_list.append('function')
    
    print(f"process_list is {process_list}")
    return process_list


corresp_format = f"""
{{
    "answer": [
        {{
            "name": (Name of the C element),
            "start_line": (Start line in the C code),
            "end_line": (End line in the C code),
            "rust_code": (Code after translation to Rust)
        }},...
    ],
    "ongoing": true or false
}}
"""


generate_format = f"""
{{
    "answer": [
        {{
            "name": (Name of the C element),
            "start_line": (Start line in the C code),
            "end_line": (End line in the C code),
            "rust_code": (Code after translation to Rust)
        }},...
    ],
    "ongoing": true or false
}}
"""


def retrieve_current_code(modified_list, raw_dir):
    with_type_A = False
    added_list = []
    template_path = 'tmp_template.json'

    for item in modified_list:
        rust_path = rust_output_dir + "/" + item['file_path']
        c_path = get_c_path(rust_path, raw_dir)
        rust_meta_data, rust_meta_path = obtain_metadata(c_path, meta_dir, True, None, "def")
        if 'start_line' in item:
            start_line = item['start_line']
        else:
            start_line = 'temp'

        print(f"c_path is {c_path}")
        print(f"rust_path is {rust_path}")
        print(f"rust_meta_path is {rust_meta_path}")
        if rust_meta_data is None: # Sometimes it fixes things like src.rs on its own
            print("rust_meta_data is None")
            write_json(template_path, modified_list)
            return template_path, with_type_A, added_list
        
        for m_item in rust_meta_data:
            if isinstance(start_line, int):
                if m_item['start_line'] <= start_line <= m_item['end_line']: # Because this might be slightly off with ctags
                    item['current_code'] = m_item['current_code']
                    item['category'] = m_item['category']
                    break
            else:
                continue

    for item in modified_list: 
        if 'current_code' in item:            
            current_path = 'tmp_current.rs'
            write_file(current_path, item['current_code'])
            line_count = count_file_lines(current_path)
            if line_count >= 300 and 'category' in item and item['category'] != 'function':
                print(f"line_count is {line_count}")
                item['format_type'] = "A"
                with_type_A = True
            
            delete_file(current_path)
        
        else:
            item['current_code'] = None

    if with_type_A:
        for item in modified_list:
            if 'format_type' not in item:
                item['format_type'] = "B"
    else:
        for item in modified_list:
            if 'format_type' in item:
                del item['format_type']
        
    for item in modified_list:
        if 'category' in item:
            del item['category']

    new_modified_list = []
    for item in modified_list:
        if 'added' in item and item['added'] is True:
            added_list.append(item)
        else:
            if 'modified_data' in item:
                del item['modified_data']
    """
    for item in modified_list:
        if 'added' in item:
            if item['added'] is False:
                new_modified_list.append(item)
                if 'modified_data' in item:
                    del item['modified_data']
            else:
                new_modified_list.append(item) # modified
                #added_list.append(item)
        else:
            new_modified_list.append(item)
            if 'modified_data' in item:
                del item['modified_data']
    modified_list = new_modified_list.copy()
    """

    for item in modified_list:
        if 'added' in item:
            del item['added']

    for item in modified_list: 
        if 'current_code' in item:
            del item['current_code']

    write_json(template_path, modified_list)

    return template_path, with_type_A, added_list


def get_file_prompt(c_path, meta_dir):
    ref_files = get_ref_files(c_path, dep_json_path)
    prompt = []

    if len(ref_files) > 0:
        prompt.extend(["The referenced information is defined as follows."])

    for ref_file in ref_files:
        ref_code = None
        rust_ref_file = get_rust_path(ref_file, map_path)

        if os.path.exists(rust_ref_file):
            ref_code = read_file(rust_ref_file)
        else:
            parent_path = get_parent_path(ref_file, map_path)
            rust_ref_file = get_rust_path(parent_path, map_path)
            ref_code = read_file(rust_ref_file)

        if ref_code is not None:
            prompt.extend([f"{ref_code}"])
    
    return prompt



def insert_modified_plain(mod):
    last_count = count_file_lines(mod['file_path'])

    print("current_code_found is True in insert_modified_data()")
    part_last_code = read_specific_lines(mod['file_path'], mod['end_line'] + 1, last_count)

    delete_lines(mod['file_path'], mod['start_line'], last_count)

    append_file(mod['file_path'], '\n')
    append_file(mod['file_path'], mod['modified_data'])

    append_file(mod['file_path'], '\n')
    append_file(mod['file_path'], part_last_code)


def insert_modified_data(mod):
    last_count = count_file_lines(mod['file_path'])

    if mod['current_code_found'] == True:
        print("current_code_found is True in insert_modified_data()")
        part_code = read_specific_lines(mod['file_path'], mod['current_end_line'] + 1, last_count)

    else:
        print("current_code_found is False in insert_modified_data()")
        part_code = read_specific_lines(mod['file_path'], mod['start_line'], last_count)

    delete_lines(mod['file_path'], mod['start_line'], last_count)

    append_file(mod['file_path'], '\n')
    data = mod['modified_data']
    if isinstance(data, (list, dict)):
        data = json.dumps(data, indent=4, ensure_ascii=False)
    append_file(mod['file_path'], data)

    append_file(mod['file_path'], '\n')
    append_file(mod['file_path'], part_code)



def insert_json_modified_data(mod): #current_code_length
    print("Updating json data")

    if isinstance(mod['modified_data'], (list, dict)):
        json_content = mod['modified_data']  # Already a Python object, no need for loads
    else:
        json_content = json.loads(mod['modified_data'])  # Use loads if it's a string

    if os.path.exists(mod['file_path']):
        existing_data = read_json(mod['file_path'])
    else:
        existing_data = []

    existing_data.extend(json_content)

    write_json(mod['file_path'], existing_data)


def remove_dup_modification(modifications):
    processed_intervals = {}
    
    for item in modifications:
        file_path = item['file_path']
        start_line = item['start_line']
        end_line = item['end_line']
        
        # Initialize the dictionary for a new file
        if file_path not in processed_intervals:
            processed_intervals[file_path] = []
            
        is_overlapping = False
        for i in range(start_line, end_line + 1):
            if i in processed_intervals[file_path]:
                is_overlapping = True
                break
        
        if not is_overlapping:
            for i in range(start_line, end_line + 1):
                processed_intervals[file_path].append(i)
        
        item['is_overlapping'] = is_overlapping
    
    new_modifications = []
    for item in modifications:
        if 'is_overlapping' in item and item['is_overlapping'] is False:
            new_modifications.append(item)
    
    return new_modifications


def get_reflect_count(count_path):
    """
    Read the count value from count_path, increment it, and save it back
    
    Args:
        count_path: Path to the file where the count is stored
        
    Returns:
        int: The count value after incrementing
    """
    # Check if the file exists
    if not os.path.exists(count_path):
        write_json(count_path, {"count": 0})
    
    # Read the file
    with open(count_path, 'r', encoding='utf-8') as f:
        content = f.read().strip()
        data = json.loads(content)
        count = data.get('count', 0)

    # Increment the count
    count += 1
    
    # Save the updated count
    data['count'] = count
    write_json(count_path, data)

    print(f"Incremented count={count} and saved to {count_path}")
    return count


# Change from the starting line to the line that needs modification
ask_template = f"""{{
    "answer" : [
        {{
            "file_path" : (file path),
            "start_line" : (starting line in the original source code that needs to be deleted or modified),
            "end_line" : (ending line in the original source code that needs to be deleted or modified),
        }},
        {{
            "file_path" : (file path),
            "start_line" : (starting line in the original source code that needs to be deleted or modified),
            "end_line" : (ending line in the original source code that needs to be deleted or modified),
        }},...
    ],
    "toml" : (if necessary)
}}
"""         

line_template = f"""{{
    "answer" : [
        {{
            "file_path" : (file path),
            "start_line" : (start line of the original Rust code to be deleted; must reflect the original range to be replaced),
            "end_line" : (end line of the original Rust code to be deleted; must reflect the original range to be replaced),
            "modified_data" : (content of the corrected code without omissions),
            "modification_part": (the number of the current part),
        }},
        {{
            "file_path" : (file path),
            "start_line" : (start line of the original Rust code to be deleted; must reflect the original range to be replaced),
            "end_line" : (end line of the original Rust code to be deleted; must reflect the original range to be replaced),
            "modified_data" : (content of the corrected code without omissions),
            "modification_part": (the number of the current part),
        }},...
    ],
    "ongoing" : true or false,
}}
"""

def get_modification_files(template_json):
    modification_files = set()
    modification_by_file = {}

    for item in template_json:
        rust_path =item['file_path']
        modification_files.add(rust_path)
        if rust_path not in modification_by_file:
            modification_by_file[rust_path] = []

        modification_by_file[rust_path].append(item)
    
    print(f"modification_files are {modification_files}")
    return list(modification_files), modification_by_file


def generate_rust_import(rust_path):

    rust_dir = rust_output_dir + "/" + "src"
    initial_path = remove_base_path(rust_path, rust_dir)

    path_parts = initial_path[:-3].split(os.sep) # get each part of the path under src
    sum_name = "use crate::" + path_parts[0]
    
    # Add remaining directories to the sum_name
    if len(path_parts) > 1:
        for i in range(1, len(path_parts)):
            sum_name += "::" + path_parts[i]
    
    sum_name += "::*;"

    sum_name = change_hyphn(sum_name)  # added
    return sum_name

def get_pure_module(rust_path):
    rust_dir = rust_output_dir + "/" + "src"
    initial_path = remove_base_path(rust_path, rust_dir)

    path_parts = initial_path[:-3].split(os.sep) # get each part of the path under src
    sum_name = "" + path_parts[0]
    
    # Add remaining directories to the sum_name
    if len(path_parts) > 1:
        for i in range(1, len(path_parts)):
            sum_name += "::" + path_parts[i]
    
    #sum_name += "::*;"

    sum_name = change_hyphn(sum_name)  # added
    return sum_name



def format_chek(modified_list, prompt, exp_data):
    return True, modified_list


####################################################
########## repair
####################################################

def get_execute_path(run_path):
    run_dir = os.path.dirname(os.path.normpath(run_path))
    execute_path = f"{run_dir}/execute.sh"

    return execute_path


modify_template = f"""{{
    "mode" : "modify_data",
    "answer" : [
        {{
            "file_path" : (file path),
            "start_line" : (start line of the original code to be deleted; must reflect the original range to be replaced),
            "end_line" : (end line of the original code to be deleted; must reflect the original range to be replaced),
            "is_deletion" : True for deletion only, False for modification,
            "no_simplification" : true if all original intended features are fully preserved, without any omissions and simplifications and placeholders. false otherwise,
            "is_JSON" :If the file_path is a JSON file, then True, otherwise False,
            "modified_data" : (content of the corrected code without any omission. Content of the corrected code as a string if is_JSON is false, or as a direct JSON object if is_JSON is true.),
            "modification_part": (the number of the current part),
        }},
        {{
            "file_path" : (file path),
            "start_line" : (start line of the original code to be deleted; must reflect the original range to be replaced),
            "end_line" : (end line of the original code to be deleted; must reflect the original range to be replaced),
            "is_deletion" : True for deletion only, False for modification,
            "no_simplification" : true if all original intended features are fully preserved, without any omissions and simplifications and placeholders. false otherwise,
            "is_JSON" :If the file_path is a JSON file, then True, otherwise False,
            "modified_data" : (content of the corrected code without any omission. Content of the corrected code as a string if is_JSON is false, or as a direct JSON object if is_JSON is true.),
            "modification_part": (the number of the current part),
        }},...
    ],
    "ongoing_in_mode" : true if the "answer" response in "modify_data" mode is long and will continue in subsequent responses. false otherwise,
    "ongoing" : true if the response will continue in a different mode. false otherwise,
    "ready_to_execute" : true if this response marks the end of a coherent modification set and it's ready to be tested; false otherwise,
    "reason" : explanatory text for the response (insert here if needed)
}}
"""

compile_template = f"""# In "read_data" mode
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
    "reason" : explanatory text for the response (insert here if needed)
}}

# In "execute_command" mode
{{
    "mode" : "execute_command",
    "answer" : shell script content to be executed,
    "ongoing_in_mode" : true if the "answer" response in "execute_command" mode is long and will continue in subsequent responses. false otherwise,
    "ongoing" : true if the response will continue in a different mode. false otherwise,
    "reason" : explanatory text for the response (insert here if needed)
}}
"""

autonomous_template = f"""# In "read_data" mode
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
            "is_deletion" : True for deletion only, False for modification,
            "no_simplification" : true if all original intended features are fully preserved, without any omissions and simplifications and placeholders. false otherwise,
            "is_JSON" :If the file_path is a JSON file, then True, otherwise False,
            "modified_data" : (content of the corrected code without any omission. Content of the corrected code as a string if is_JSON is false, or as a direct JSON object if is_JSON is true.),
            "modification_part": (the number of the current part),
        }},
        {{
            "file_path" : (file path),
            "start_line" : (start line of the original code to be deleted; must reflect the original range to be replaced),
            "end_line" : (end line of the original code to be deleted; must reflect the original range to be replaced),
            "is_deletion" : True for deletion only, False for modification,
            "no_simplification" : true if all original intended features are fully preserved, without any omissions and simplifications and placeholders. false otherwise,
            "is_JSON" :If the file_path is a JSON file, then True, otherwise False,
            "modified_data" : (content of the corrected code without any omission. Content of the corrected code as a string if is_JSON is false, or as a direct JSON object if is_JSON is true.),
            "modification_part": (the number of the current part),
        }},...
    ],
    "ongoing_in_mode" : true if the "answer" response in "modify_data" mode is long and will continue in subsequent responses. false otherwise,
    "ongoing" : true if the response will continue in a different mode. false otherwise,
    "ready_to_execute" : true if this response marks the end of a coherent modification set and it's ready to be tested; false otherwise,
    "reason" : explanatory text for the response (insert here if needed)
}}

# In "execute_command" mode
{{
    "mode" : "execute_command",
    "answer" : shell script content to be executed,
    "ongoing_in_mode" : true if the "answer" response in "execute_command" mode is long and will continue in subsequent responses. false otherwise,
    "ongoing" : true if the response will continue in a different mode. false otherwise,
    "reason" : explanatory text for the response (insert here if needed)
}}
"""


# "overwrite_all" : Flag for full file modification. If true, overwrites the whole file; if false, modifies only the specified lines. To minimize the number of tokens used, please basically write specific lines with this flag 'false'.
unimpl_template = f"""[
    {{
        "name": "(name of the corresponding element in the translated Rust code)",
        "rust_file_path": "(path to the Rust file where the corresponding element is located)",
        "rust_start_line": "(starting line number of the Rust element)",
        "rust_end_line": "(ending line number of the Rust element)"
        "have_correspondence" : True if there is a corresponding item in the original C code. False otherwise,
        "c_name": "(name of the element (function and so on) in the original C code)",
        "c_file_path": "(path to the C file where the element is located)",
        "no_equivalent_reason" : "// reason why it has not Rust correspondence",
    }},
    {{
        "name": "(name of the corresponding element in the translated Rust code)",
        "rust_file_path": "(path to the Rust file where the corresponding element is located)",
        "rust_start_line": "(starting line number of the Rust element)",
        "rust_end_line": "(ending line number of the Rust element)"
        "have_correspondence" : True if there is a corresponding item in the original C code. False otherwise,
        "c_name": "(name of the element (function and so on) in the original C code)",
        "c_file_path": "(path to the C file where the element is located)",
        "no_equivalent_reason" : "// reason why it has not Rust correspondence",
    }},...
]
"""

def create_rust_build_path(run_path) -> str:  # , target_directory: str
    # Create the script content
    target_directory = "trans_rust"
    #  --manifest-path={target_directory}/Cargo.toml


    # {target_directory}
    script_content = f"""#!/bin/bash

# Exit immediately if any command fails
set -e

# Execute the build
echo "Building Rust project..."
RUSTFLAGS="-Awarnings" cargo build --release --manifest-path=Cargo.toml

# Check build result
if [ $? -eq 0 ]; then
   echo "Build completed successfully"
   exit 0
else
   echo "Build failed"
   exit 1
fi
"""
    
    try:
        # Create the script file
        with open(run_path, 'w') as f:
            f.write(script_content)
        
        # Grant execute permission
        os.chmod(run_path, 0o755)
        
        print(f"Created Rust build script at: {run_path}")
        return run_path
        
    except Exception as e:
        print(f"Error creating Rust build script: {str(e)}")
        raise


# cargo modules structure
def get_cargo_modules(execute_dir) -> str:
    """
    Executes cargo modules structure command and returns the output
    """
    try:
        # Get and copy the current environment variables
        my_env = os.environ.copy()
        # Set the NO_COLOR environment variable
        my_env["NO_COLOR"] = "1"

        result = subprocess.run(
            ['cargo', 'modules', 'structure'],
            #['cargo', 'modules', 'structure'],  # , '--no-color'
            capture_output=True,
            text=True,
            check=True,
            cwd=execute_dir,
            env=my_env  # Specify environment variables
        )
        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"Error executing cargo modules: {e}")
        return ""
    except FileNotFoundError:
        print("cargo-modules is not installed. Please install it using:")
        print("cargo install cargo-modules")
        return ""


def transform_tmp(tmp_rust_path, label):    
    json_data = read_json(tmp_rust_path)

    #if label == "function":
    element_count = 0
    for item in json_data:
        item["element_id"] = element_count
        item["have_correspondence"] = None
        item["no_equivalent_reason"] = "// reason why it has not Rust correspondence",
        element_count += 1

    return tmp_rust_path


    element_count = 0
    new_data = []
    for item in json_data:
        new_item = {
            "element_id" : element_count,
            "c_name" : item['name'],
            "c_start_line" : item['start_line'],
            "c_end_line" : item['end_line'],
            "rust_name" : None,
            "rust_start_line" : None,
            "rust_end_line" : None,
            "have_correspondence" : None,
            "no_equivalent_reason" : "// reason why it has not Rust correspondence",
        }
        new_data.append(new_item)

        element_count += 1

    write_json(tmp_rust_path, new_data)

    return tmp_rust_path


# Remove unnecessary information from the C-to-Rust conversion intermediate data and format it into a simpler structure
def reverse_tmp(answer_path, mod_rust_path, label, database_dir):
    
    json_data = read_json(answer_path)
    if json_data is None:
        return 

    for item in json_data:
        if 'element_id' in item:
            del item['element_id']
        if 'no_equivalent_reason' in item and 'have_correspondence' in item and item['have_correspondence'] is False and item['no_equivalent_reason'] is not None:
            item['rust_code'] = item['no_equivalent_reason']
    write_json(answer_path, json_data)
    return

    new_data = []
        
    for item in json_data:
        rust_code = None 
        if item["rust_start_line"] is not None and item["rust_end_line"] is not None: # There may be cases where correspondence does not exist
            rust_code = read_specific_lines(mod_rust_path, item["rust_start_line"], item["rust_end_line"])
        
        else:
            rust_code = item["no_equivalent_reason"]

        new_item = {
            "name" : item['c_name'],
            "start_line" : item['c_start_line'],
            "end_line" : item['c_end_line'],
            "rust_code" : rust_code
        }
        new_data.append(new_item)

    write_json(f"{database_dir}/initial.json", json_data)
    write_json(f"{database_dir}/initial2.json", new_data)

    write_json(answer_path, new_data)



def update_judge_dict(rust_path, judge_count, judge_path):
    if os.path.exists(judge_path):
        judge_json = read_json(judge_path)
    else:
        judge_json = {}
    
    if rust_path not in judge_json:
        judge_json[rust_path] = []

    judge_json[rust_path].append(judge_count) # Since the same file may go through compilation multiple times

    write_json(judge_path, judge_json)


def create_execute_path(execute_path):
    if not os.path.exists(execute_path):
        create_file(execute_path)
    else:
        delete_file(execute_path)  # clear the file
        create_file(execute_path)
    os.chmod(execute_path, 0o755)


def repair_execute(repair_target, interface):

    build_path = interface.build_path
    rust_build_path = interface.rust_build_path
    run_test_path = interface.run_test_path
    run_all_path = interface.run_all_path
    
    rust_path = interface.rust_path
    repair_count = interface.repair_count
    rust_edition = interface.rust_edition

    #repair_target = interface.repair_target
    work_dir = interface.work_dir
    target_dir = interface.target_dir
    raw_dir = interface.raw_dir
    database_dir = interface.database_dir
    rust_output_dir = interface.rust_output_dir
    target_path = interface.target_path

    llm_interface = interface.llm_interface
    output_max = llm_interface.output_max
    
    run_path = run_all_path
    
    rust_c_path = interface.rust_c_path
    c_rust_path = interface.c_rust_path

    rust_c_map = read_json(rust_c_path)
    c_rust_path = read_json(c_rust_path)
    
    modified_c_keys = set()
    modified_rust_lines = []

    execute_path = f"{work_dir}/execute.sh"
    create_execute_path(execute_path)
    execute_dir = os.path.dirname(os.path.normpath(execute_path))

    if repair_target == "build":
        # From build
        entry = interface.entry
        meta_dir = interface.meta_dir
        dep_json_path = interface.dep_json_path
        exp_data = interface.exp_data
        rust_path = interface.rust_path
        lib_path = interface.lib_path
    
    elif repair_target == "compile":
        # From compile
        add_prompt = interface.add_prompt
        c_path = interface.c_path
        meta_dir = interface.meta_dir
        dep_json_path = interface.dep_json_path
        div_start_line = interface.div_start_line
        exp_data = interface.exp_data
        before_count = interface.before_count

    elif repair_target == "ask_generates":
        answer_path = interface.answer_path
        rust_path = interface.rust_path
        meta_dir = interface.meta_dir
        dep_json_path = interface.dep_json_path
        exp_data = interface.exp_data
        modified_files = interface.modified_files
        conds_data = read_json(interface.conds_status_path)
        lib_path = interface.lib_path


    elif repair_target == "ask_correspondence":
        c_path = interface.c_path
        meta_dir = interface.meta_dir
        answer_path = interface.answer_path
        rust_path = interface.rust_path
        meta_dir = interface.meta_dir
        dep_json_path = interface.dep_json_path
        exp_data = interface.exp_data
        modified_files = interface.modified_files

    # start iteration
    mode = None
    execute_error = None
    execute_out = None
    read_prompt = None

    error = True # Assuming errors will exist
    ongoing_flag = False
    mode = None

    editied_files = []
    judge_count = 0

    functions = parse_function_info(target_path, work_dir, user_id)

    while (1):
        if repair_count == REPAIR_MAX:
            print(f"Force to finish. Hit the REPAIR_MAX ({REPAIR_MAX}).")
            iteration_dict[rust_path] = repair_count
            sys.exit(1)  #return True
        
        if mode != "read_data":
            if (repair_count != 1 and repair_target in ["build", "CARGO", "BUILD", "LIB"]) or repair_target == "compile": #if (repair_count != 1 and (repair_target == "build" or repair_target == "compile")):
                error, std_out, repair_count = run_script(run_path, 100, True, None, "both", None, repair_count, None, None, mode)
                judge_count += 1
                print(f"Judging at run_script: error: {error}")

        # if ongoing_flag is False and error is None and mode != "read_data": # This condition is tricky, because ongoing_flag may not always work correctly.
        #     break

        if repair_target == "ask_generates" or repair_target == "ask_correspondence":
            if repair_count != 1 and mode != "read_data" and ongoing_flag is False:
                #judge_count += 1
                break

        print(f"Judging at {repair_count}: run_path: {run_path} mode: {mode}, ongoing_flag: {ongoing_flag}, error: {error}")
        if error is None and mode != "read_data":  # This feels like a big change though  # if error is None and mode != "read_data" and ongoing_flag is False:
            break

        if repair_target == "compile":
            ref_files = [] # prompt = [] # It's an option to inherit the prompt, but initializing here for now
            add_prompt = []

            if DEBUG_LLM:
                iteration_dict[rust_path] = repair_count
                return

            if repair_count == 1:
                if before_count is None:
                    repair_count = 1 # One round of convert_llm
                else:
                    repair_count = before_count

                modified_file_list = []
                toml_response = None

                prompt = []

                if FFI_STRATEGY == "preserve":
                    prompt.extend([f"When running the Rust program in {rust_output_dir}, the following error occurs. Please fix the Rust program in {rust_output_dir} to resolve the error.",
                            "When responding, follow the response rules below and select only one of the three Response modes.",
                            "",
                            "## Response rules", 
                            f"- Identify and fix the file that fundamentally resolves the error, not limited to {rust_path}. If necessary, use read_data mode to check the program's content.",
                            #"- In modification, do not use unsafe or raw pointers. Instead, please use safe Rust types and operations including custom structs, enums, Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.", #"- NEVER use unsafe or raw pointers. Only use safe Rust types and operations: Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.",
                            "- In modification, do not use unsafe, raw pointers, or manual memory management as much as possible.",
                            "    - EXCEPTION: Permit minimal unsafe blocks strictly limited to the following two categories.",
                            "                  1. Stub implementation of the FFI boundary functions (specified in ## FFI boundary functions below): These C functions are being replaced by Rust. These are declared as extern C fn with #[unsafe(no_mangle)] so that C code can call the Rust replacement. Stub implementation of the FFI boundary functions MUST remain unchanged until you are explicitly instructed to replace them with the actual implementation. Do NOT implement the actual logic of it.", # Note that even inside FFI boundary functions, extract logic into safe Rust helper functions.",
                            "                  2. Global variables: C global variables shared across the boundary, accessed through unsafe extern C static declarations with getter and setter functions.",
                            #"                  3. cfg attribute flags: Conditional compilation flags registered as cargo rustc cfg, used with #[cfg(has_FLAG_NAME)] attributes.",
                            #"                  4. Independent constant macros: C macro constants generated by bindgen in bindings.rs, used directly by name without redefinition.",
                            "    - For everything else, write in safe Rust without calling C functions through FFI.",
                            # f"- The code content should be complete, without omissions, and should include entire units such as functions or data types, starting from lines without indentation.", # Ensure the code is complete without omission and includes entire units like functions or data types that can be parsed by ctags.
                            # "- The code content should be ready to run as-is, without omissions. If the token limit is likely to be exceeded, please split the response into multiple parts.", # but it's omitted for brevity as it would exceed the token limit.
                            "- When representing backslashes as byte literals, escape the backslash twice in the source code, and also escape it again in the byte literal, resulting in four backslashes (double backslashes).",
                            "- When representing backslashes as character literals, escape the backslash once in the source code and again in the character literal, resulting in two backslashes.",
                            f"- When making modifications, consider the directory structure where the Rust program ({rust_output_dir}) is located.",
                            f"- The shell script code to execute the Rust program in {rust_output_dir} is in the file at {run_path} and is executed with ./{run_path}",
                            #"- Declare all items (structs, enums, functions, constants, etc.) with pub (public) so they can be imported from other modules.",
                            "- Define all functions and types at the top level. Avoid using the mod keyword for module partitioning and avoid defining methods for types (within impl blocks).", #"- Define all functions and types at the top level without using the mod keyword.",
                            # "- When writing functions, first add 'use tracing::info;' at the top of the file, then add log outputs using the info!() macro in each function so that the values of the arguments and the return values can be traced as follows:",
                            # "    - Just after function entry, log the argument values with: \"Enter_log::: variable_name1 = value1, variable_name2 = value2, ...\", and just before function exit, log the return values with: \"Return_log::: variable_name1 = value1, variable_name2 = value2, ...\"",
                            # "    - If an argument or return value is a pointer, please log its value rather than its address.",
                            # "    - If there are no arguments or return values, you do not need to include an info!() log output.",
                            # "    - For any argument or return value that does not implement the Debug trait (and thus cannot be used with the tracing crate), log its value as <unprintable>.",
                            # "- To ensure that the tracing crate can be used, please implement the Debug trait for all data types defined in the code that are used as function argument or return types.",
                            f"- Use Rust {rust_edition} edition.",
                            #"- For strings that are treated as literal strings in the original C code, ensure that in the translated Rust code these strings are identical for equality checks -this includes preserving case sensitivity (uppercase/lowercase) and making sure every single character matches perfectly. Similarly, any strings originally printed via printf should be translated into Rust exactly as they appear in the original C code.",                   
                            ])

                else:
                    prompt.extend([f"When running the Rust program in {rust_output_dir}, the following error occurs. Please fix the Rust program in {rust_output_dir} to resolve the error.", 
                            "When responding, follow the response rules below and select only one of the three Response modes.", 
                            "",
                            "## Response rules", 
                            f"- Identify and fix the file that fundamentally resolves the error, not limited to {rust_path}. If necessary, use read_data mode to check the program's content.", 
                            #"- In modification, do not use unsafe or raw pointers. Instead, please use safe Rust types and operations including custom structs, enums, Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.", #"- NEVER use unsafe or raw pointers. Only use safe Rust types and operations: Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.",
                            "- In modification, do not use unsafe, raw pointers, or manual memory management as much as possible.",
                            "    - EXCEPTION: Permit minimal unsafe blocks strictly limited to the following two categories.",
                            #"                  1. Stub implementation of the FFI boundary functions (specified in ## FFI boundary functions below): These C functions are being replaced by Rust. These are declared as extern C fn with #[unsafe(no_mangle)] so that C code can call the Rust replacement. Stub implementation of the FFI boundary functions MUST remain unchanged until you are explicitly instructed to replace them with the actual implementation. Do NOT implement the actual logic of it.", # Note that even inside FFI boundary functions, extract logic into safe Rust helper functions.",
                            "               - The rust_main_<identifer> entry point: declared as pub extern \"C\" fn rust_main_<identifer>() with #[no_mangle] so that C main() can call it. This is the only FFI boundary in this project.",
                            #"               - Global variables: C global variables shared across the boundary, accessed through unsafe extern C static declarations with getter and setter functions.",
                            #"                  3. cfg attribute flags: Conditional compilation flags registered as cargo rustc cfg, used with #[cfg(has_FLAG_NAME)] attributes.",
                            #"                  4. Independent constant macros: C macro constants generated by bindgen in bindings.rs, used directly by name without redefinition.",
                            "    - For everything else, write in safe Rust without calling C functions through FFI.",
                            # f"- The code content should be complete, without omissions, and should include entire units such as functions or data types, starting from lines without indentation.", # Ensure the code is complete without omission and includes entire units like functions or data types that can be parsed by ctags.
                            # "- The code content should be ready to run as-is, without omissions. If the token limit is likely to be exceeded, please split the response into multiple parts.", # but it's omitted for brevity as it would exceed the token limit.
                            "- When representing backslashes as byte literals, escape the backslash twice in the source code, and also escape it again in the byte literal, resulting in four backslashes (double backslashes).",
                            "- When representing backslashes as character literals, escape the backslash once in the source code and again in the character literal, resulting in two backslashes.",
                            f"- When making modifications, consider the directory structure where the Rust program ({rust_output_dir}) is located.",
                            f"- The shell script code to execute the Rust program in {rust_output_dir} is in the file at {run_path} and is executed with ./{run_path}",
                            #"- Declare all items (structs, enums, functions, constants, etc.) with pub (public) so they can be imported from other modules.",
                            "- Define all functions and types at the top level. Avoid using the mod keyword for module partitioning and avoid defining methods for types (within impl blocks).", #"- Define all functions and types at the top level without using the mod keyword.",
                            # "- When writing functions, first add 'use tracing::info;' at the top of the file, then add log outputs using the info!() macro in each function so that the values of the arguments and the return values can be traced as follows:",
                            # "    - Just after function entry, log the argument values with: \"Enter_log::: variable_name1 = value1, variable_name2 = value2, ...\", and just before function exit, log the return values with: \"Return_log::: variable_name1 = value1, variable_name2 = value2, ...\"",
                            # "    - If an argument or return value is a pointer, please log its value rather than its address.",
                            # "    - If there are no arguments or return values, you do not need to include an info!() log output.",
                            # "    - For any argument or return value that does not implement the Debug trait (and thus cannot be used with the tracing crate), log its value as <unprintable>.",
                            # "- To ensure that the tracing crate can be used, please implement the Debug trait for all data types defined in the code that are used as function argument or return types.",
                            f"- Use Rust {rust_edition} edition.",
                            #"- For strings that are treated as literal strings in the original C code, ensure that in the translated Rust code these strings are identical for equality checks -this includes preserving case sensitivity (uppercase/lowercase) and making sure every single character matches perfectly. Similarly, any strings originally printed via printf should be translated into Rust exactly as they appear in the original C code.",                   
                    ])


                prompt.extend(["\n## FFI boundary functions:"])
                for func in functions:
                    prompt.append(f"   - {func['name']} (from {func['file_path']}, line {func['start_line']})") # lines {func['start_line']}-{func['end_line']})")
                
                # Important! Additional information
                prompt.extend(add_prompt)
    
            else:
                if error is None:
                    prompt = []
                    prompt = [f"Continue providing solutions to resolve errors that occur when calling the Rust library {rust_output_dir} by running {run_test_path}."] 

                else:
                    prompt = []
                    prompt.extend([f"Please provide a response with JSON data to resolve the error that occurs when calling the Rust library {rust_output_dir} by running {run_test_path}.",
                                    "Please follow the rules below when providing your answer.",
                                    "",
                                    "## Response rules",
                                    f"- Identify and fix the file that fundamentally resolves the error, not limited to {rust_path}. If necessary, use read_data mode to check the program's content.", 
                                    "- Since we will ask about the actual modifications later, for now, please only specify the \"start_line\" and \"end_line\" that need modification in modify_data mode. Do not write \"modified_data\"",
                                    #"- Because we have the goal to have memory-safe Rust code to enhance security, please avoid using unsafe, and use the Rust standard library or crates to achieve equivalent functionality in a safe manner.",
                                    #"- When translating C raw pointers to Rust, use appropriate safe representations such as references, slices, Option, Box, Rc, Arc, etc.",
                                    #f"- The content of the corrected code in 'modified_data' must not contain any omissions because we execute the provided code directly. Do NOT use comments like \"// Implementation omitted for brevity\" or similar.",
                                    #"- Additionally, 'modified_data' must ensure proper indentation as the code will be copied and pasted directly between start_line and end_line.", # Ensure the code can be compiled.
                                    #"- If an error occurs when representing a backslash as a byte literal, escape the backslash in the source code and also in the byte literal by using four backslashes (double backslashes).",
                                    #"- If an error occurs when representing a backslash as a character literal, escape the backslash in the source code and also in the character literal by using two backslashes.",
                                    # "If the 'modified_data' for a single modification point is very long, split the response into parts with no more than 200 lines of corrected code per response and include the boolean value 'segmented' as shown below, responding over multiple submissions.", # added for seg
                                    # "Encode the modified code in Base64, so it can be decoded with base64.b64decode(modified_data).decode('utf-8'), and enter it in the value of the 'modified_data' key in JSON format.",
                                    # "Please enter the modified code in the value of the 'modified_data' key for the corresponding parsable_id in JSON format.",
                                    # "When making modifications, avoid using unsafe for external function calls and, as much as possible, use the Rust standard library or crates to achieve equivalent functionality in a safe manner.", # added A
                                    f"- To avoid hitting the token limit, keep the JSON data included in one response within {output_max} tokens.", # If it's a long response,
                                    # "If the response is split into multiple parts and there is still remaining JSON data, write a boolean value of True for the 'ongoing' key. If the JSON data is the final part, write a boolean value of False for the 'ongoing' key.",
                                    "- The modified code must be complete and ready to copy-paste for execution, with no omissions. If the JSON data in one response is likely to exceed the token limit, split the response into multiple parts.",
                                    "- If this is the final part of the JSON data, set the 'ongoing' key to a boolean value of False. If there is remaining JSON data, set the 'ongoing' key to a boolean value of True."
                                    ])

                    if ongoing_flag is False:
                        # Important! Added information
                        prompt.extend(add_prompt)

        # Response format
        print(f"ongoing_flag is {ongoing_flag}") 
        if ongoing_flag is False:
            print(repair_count)
            print(repair_target)

            if repair_target == "compile" or (repair_target == "build" and repair_count != 1):
                prompt.extend(["",
                                "## Response modes:",
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
                                f"- This executes separately from {run_path}. If not necessary beyond {run_path}, you do not need to include it in the response.",
                                "- Put the shell script code to be executed in the \"answer\" field of the JSON format data.",
                                f"- The answered shell script code will be saved in the shell script file at {execute_path} and executed in the {execute_dir} directory.",
                                "- The execution of ./execute.sh should not have any arguments.",
                                "- The shell script can include multiple commands.",
                            ])

                prompt.extend(["\n## Response format", "- In summary, please respond in the following JSON format:"]) 
                prompt.extend([compile_template])

            else:

                prompt.extend(["",
                                "## Response modes:",
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
                                "- To accurately identify the parts to be modified, make sure to always read the target file in read_data mode before executing modify_data mode.",
                                "- Please insert the filename, start line, and end line of the section to be deleted into the \"file_path\", \"start_line\", and \"end_line\" keys in the JSON data.",                                              
                                "- Then, insert the new content that should be inserted at that [start_line] into the value of the \"modified_data\" key.",
                                "- Detailed modification process is as follows. Please carefully write start_line, end_line and modified_data considering the process:",
                                "    1. All code in the specified range (from [start_line] to [end_line]) will be completely deleted.",
                                "    2. The content you provide in \"modified_data\" will be inserted at [start_line].",
                                "    3. All code from [end_line + 1] onwards will remain unchanged and be appended after your modified_data.",
                                "- Please use the exact line numbers and indentation levels shown on the left side of the code (Line X [Y], where X is the line number and Y is the indentation level) for start_line, end_line and modified_data.",
                                "- In case the modification content (modified_data) for a single range (start_line-end_line) is too long to include in one entry:",
                                "    - Please split it across multiple answer entries.",
                                "    - Each of these answer entries should maintain the same file_path, start_line, and end_line values",
                                "    - Include modification_part representing the number of the current part in each entry to track the split.",
                                #"    - Include modification_part information in each entry to track the split:",
                                #"        - current: the number of the current part:",
                                #"        - total: the total number of parts",
                                "    - please remain ongoing_in_mode and ongoing flags true until all parts are delivered",
                                #"- Do not propose multiple modifications for the same line.",
                                #"- Do not split your modifications across multiple entries in the answer array when they target the exact same line range. For each unique (start_line, end_line) pair, there should be only ONE modification entry in your answer.",
                                "- Set the ready_to_execute flag to True if this response marks the end of a coherent modification set and it's ready to be tested",
                                #"- The 'Line X:' prefix is not part of the actual code - they're just line indicators. When providing modified_data, please use the correct indentation from the original code, ignoring the prefix.",
                                #"- Insert the filename, start line, and end line of the change location to be inserted into the values of the \"file_path\", \"start_line\", and \"end_line\" keys in JSON format data.",
                                f"- For file_path, write a relative path in the format '{work_dir}/path/to/file'", #f"- For file_path, write a relative path starting from {work_dir}.",
                                "- Insert appropriate indentation in modified_data so that it can be executed correctly when copied and pasted into the original code's location from start_line to end_line.",
                                "- \"modified_data\" must not contain any omissions and must strictly maintain the appropriate indentation, as it will be directly inserted and executed in the original code.",
                                "- Ensure that \"modified_data\" follows the exact indentation level [Y] shown for each line in the original code.",
                                "- If you want to only perform deletion without inserting into a specific location in the existing specified file path, set the value of 'is_deletion' to True.",
                                #"- If you want to overwrite the entire file rather than just modifying the specified line range, set the value of 'overwrite_all' to True.",
                                "- Set the value of \"no_simplification\" to True if the functionality intended before modification exists completely without any omission and simplification. Set it to False otherwise.",
                                "- If the target file for editing is a JSON file, set the \"is_JSON\" flag to True and insert the modified JSON data into \"modified_data\"",
                                #""
                                #"- Since we will ask about the actual modifications later, for now, please only specify the \"start_line\" and \"end_line\" that need modification. Do not write \"modified_data\".",
                                "",
                                "3. In 'execute_command' mode:",
                                "### Purpose:",
                                "- Executes the provided shell script code.",
                                "### Format:",
                                f"- This executes separately from {run_path}. If not necessary beyond {run_path}, you do not need to include it in the response.",
                                "- Put the shell script code to be executed in the \"answer\" field of the JSON format data.",
                                f"- The answered shell script code will be saved in the shell script file at {execute_path} and executed in the {execute_dir} directory.",
                                "- The execution of ./execute.sh should not have any arguments.",
                                "- The shell script can include multiple commands."
                            ])

                prompt.extend(["\n## Response format", "- In summary, please respond in the following JSON format:"])
                prompt.extend([autonomous_template])
                #prompt.extend([see_template])

                if W_O_DEP:
                    prompt.extend([f"- Now we have a special rule. As only modifications to {reflect_path} are possible, please specify the parts of {reflect_path} that need to be corrected."]) 
                
                
            rust_code = get_lined_code(rust_path, work_dir)
            prompt.extend(["", f"## Code in {rust_path}:", rust_code])

            #if repair_count != 1 and repair_target == "build":
                #prompt.extend(["Execution result of the program when calling the Rust function:", rust_flow])

            """
            if repair_count != 1 and repair_target == "compile":
                test_code = get_lined_code(rust_path)

                prompt.extend(["## Execution result of the C program:", c_flow])
                prompt.extend(["## Execution result of the program when calling the Rust function:", rust_flow])

                rsp_json = ask_json(prompt)

                # std_err, std_out = run_script()
                # rust_flow = get_flow(std_out)

                result = compare_result(c_flow, rust_flow, meta_dir)
            """
        else:
            prompt.extend(["\n## Response format", "Continue responding in the following JSON format:"])
            prompt.extend([compile_template])


        # Display the previous execution
        if execute_error is not None or execute_out is not None:
            if execute_out is not None:
                #prompt.extend(["", "- The result executed in execute_command mode is as follows:"])
                prompt.extend(["## Execution result:",
                             "The result executed in execute_command mode is as follows:"
                            f"{execute_out}",
                            ""])

            if execute_error is not None:
                prompt.extend(["## Execution result error: ",
                            "The result error executed in execute_command mode is as follows:"
                            f"{execute_error}"])
            execute_error = None # Initialization
            execute_out = None

        
        # Display the previous read_data mode
        if read_prompt is not None:
            prompt.extend(["", "## Response to the previous request:"])
            prompt.extend(read_prompt)
            read_prompt = None # Initialization

        
        # Display errors & display additional information
        if (repair_target == "build" and repair_count == 1) or ongoing_flag is False:
            if error is not None and error is not True:
                prompt.extend(["", "## Error:", error])

            if repair_target == "compile" or repair_target == "build":
                prompt.extend(["", "## Directory structure of the target Rust program:"])
                directory_structure = get_dir_struct("translation", work_dir, None) 
                prompt.extend([directory_structure, ""])

                if not W_O_DEP:
                    prompt.extend(["", "## Module structure of the target Rust program:"])
                    structure = get_cargo_modules(rust_output_dir)
                    prompt.extend([structure, ""])


            
        ################################################
        prompt = adjust_prompt(prompt)
        print("-------------------------")

        print(f"repair_target: {repair_target}")

        if repair_count == 1:
            rsp_json = ask_llm(prompt, "init", llm_interface)
        else:
            delete_file(execute_path)
            create_file(execute_path)
            os.chmod(execute_path, 0o755)
            
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

            if 'ongoing' in rsp_json:
                ongoing_flag = rsp_json['ongoing']

            if 'ongoing_in_mode' in rsp_json:
                ongoing_in_mode_flag = rsp_json['ongoing_in_mode']

            print(f"ongoing_flag at location 1 is {ongoing_flag}")
            print(f"ongoing_in_mode_flag at location 1 is {ongoing_in_mode_flag}")

            if 'mode' in rsp_json:
                mode = rsp_json['mode']

                if mode == 'modify_data':
                    if 'answer' in rsp_json:
                        modified_list = rsp_json['answer'] # It might be okay to put individually converted ones here
                        if not isinstance(modified_list, list):
                            modified_list = [modified_list]
                        sum_modified_list.extend(modified_list)

                        """
                        if repair_target == "build" and repair_count == 1: ##repair_target != "compile":
                            sum_modified_list.extend(modified_list)
                        
                        else:
                            if ongoing_in_mode_flag is True:
                                sum_modified_list.extend(modified_list)
                        """
                        if not(repair_target == "build" and repair_count == 1):  ## if not(repair_target == "build" and repair_count == 1) and ongoing_in_mode_flag is False:

                            sequences = []
                            seen_sequences = set()
                            for mod in sum_modified_list: #modified_list:
                                if mod['file_path'] not in seen_sequences:
                                    correct_path = mod['file_path']
                                    if not os.path.exists(mod['file_path']):
                                        correct_path = find_matching_path(work_dir, mod['file_path'])
                                    sequences.append(correct_path) #mod['file_path'])
                                    seen_sequences.add(correct_path) #mod['file_path'])

                            seq_string = ""
                            i = 0
                            for seq_path in sequences:
                                if i != 0:
                                    seq_string += ", "
                                seq_string += seq_path
                                i += 1


                            prompt = []

                            if FFI_STRATEGY == "preserve":
                                prompt.extend([f"Please provide the actual modified_data for the previously identified locations in {seq_string} file using modify_data mode.",
                                                "",
                                                "## Response rules",
                                                "- Please insert the filename, start line, and end line of the section to be deleted into the \"file_path\", \"start_line\", and \"end_line\" keys in the JSON data.",
                                                "- Then, insert the new content that should be inserted at that [start_line] into the value of the \"modified_data\" key.",
                                                "- Detailed modification process is as follows. Please carefully write start_line, end_line and modified_data considering the process:",
                                                "    1. All code in the specified range (from [start_line] to [end_line]) will be completely deleted.",
                                                "    2. The content you provide in \"modified_data\" will be inserted at [start_line].",
                                                "    3. All code from [end_line + 1] onwards will remain unchanged and be appended after your modified_data.",
                                                "- Please use the exact line numbers and indentation levels shown on the left side of the code (Line X [Y], where X is the line number and Y is the indentation level) for start_line, end_line and modified_data.",
                                                "- In case the modification content (modified_data) for a single range (start_line-end_line) is too long to include in one entry:",
                                                "    - Please split it across multiple answer entries.",
                                                "    - Each of these answer entries should maintain the same file_path, start_line, and end_line values",
                                                "    - Include modification_part representing the number of the current part in each entry to track the split.",
                                                #"    - Include modification_part information in each entry to track the split:",
                                                #"        - current: the number of the current part:",
                                                #"        - total: the total number of parts",
                                                "    - please remain ongoing_in_mode and ongoing flags true until all parts are delivered",
                                                #"- Do not propose multiple modifications for the same line.",
                                                #"- Do not split your modifications across multiple entries in the answer array when they target the exact same line range. For each unique (start_line, end_line) pair, there should be only ONE modification entry in your answer.",
                                                "- Set the ready_to_execute flag to True if this response marks the end of a coherent modification set and it's ready to be tested",
                                                #"- The 'Line X:' prefix is not part of the actual code - they're just line indicators. When providing modified_data, please use the correct indentation from the original code, ignoring the prefix.",
                                                f"- For file_path, write the relative path starting from {work_dir}.",
                                                "- Insert appropriate indentation in modified_data so that it can be executed correctly when copied and pasted into the original code's location from start_line to end_line.",
                                                "- \"modified_data\" must not contain any omissions and must strictly maintain the appropriate indentation, as it will be directly inserted and executed in the original code.",
                                                "- Ensure that \"modified_data\" follows the exact indentation level [Y] shown for each line in the original code.",
                                                "- If you want to only perform deletion without inserting into a specific location in the existing specified file path, set the value of 'is_deletion' to True.",
                                                #"- If you want to overwrite the entire file rather than just modifying the specified line range, set the value of 'overwrite_all' to True.",
                                                "- Set the value of \"no_simplification\" to True if the functionality intended before modification exists completely without any omission and simplification. Set it to False otherwise.",
                                                "- If the target file for editing is a JSON file, set the \"is_JSON\" flag to True and insert the modified JSON data into \"modified_data\"",
                                                #""
                                                #"- In modification, do not use unsafe or raw pointers. Instead, please use safe Rust types and operations including custom structs, enums, Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.", #"- NEVER use unsafe or raw pointers. Only use safe Rust types and operations: Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.", "- In modifications, please avoid using unsafe, and use the Rust standard library or crates to achieve equivalent functionality in a safe manner.",
                                                "- In modification, do not use unsafe, raw pointers, or manual memory management as much as possible.",
                                                "    - EXCEPTION: Permit minimal unsafe blocks strictly limited to the following two categories.",
                                                "                  1. Stub implementation of the FFI boundary functions (specified in ## FFI boundary functions below): These C functions are being replaced by Rust. These are declared as extern C fn with #[unsafe(no_mangle)] so that C code can call the Rust replacement. Stub implementation of the FFI boundary functions MUST remain unchanged until you are explicitly instructed to replace them with the actual implementation. Do NOT implement the actual logic of it.", # Note that even inside FFI boundary functions, extract logic into safe Rust helper functions.",
                                                "                  2. Global variables: C global variables shared across the boundary, accessed through unsafe extern C static declarations with getter and setter functions.",
                                                #"                  3. cfg attribute flags: Conditional compilation flags registered as cargo rustc cfg, used with #[cfg(has_FLAG_NAME)] attributes.",
                                                #"                  4. Independent constant macros: C macro constants generated by bindgen in bindings.rs, used directly by name without redefinition.",
                                                "    - For everything else, write in safe Rust without calling C functions through FFI.",
                                                "- When representing backslashes as byte literals, escape the backslash twice in the source code, and also escape it again in the byte literal, resulting in four backslashes (double backslashes).",
                                                "- When representing backslashes as character literals, escape the backslash once in the source code and again in the character literal, resulting in two backslashes.",
                                                "- Please ensure that all function names have the prefix \"rust_\".",
                                            ])
                            
                            else:
                                prompt.extend([f"Please provide the actual modified_data for the previously identified locations in {seq_string} file using modify_data mode.", 
                                                "",
                                                "## Response rules",
                                                "- Please insert the filename, start line, and end line of the section to be deleted into the \"file_path\", \"start_line\", and \"end_line\" keys in the JSON data.",
                                                "- Then, insert the new content that should be inserted at that [start_line] into the value of the \"modified_data\" key.",
                                                "- Detailed modification process is as follows. Please carefully write start_line, end_line and modified_data considering the process:",
                                                "    1. All code in the specified range (from [start_line] to [end_line]) will be completely deleted.",
                                                "    2. The content you provide in \"modified_data\" will be inserted at [start_line].",
                                                "    3. All code from [end_line + 1] onwards will remain unchanged and be appended after your modified_data.",
                                                "- Please use the exact line numbers and indentation levels shown on the left side of the code (Line X [Y], where X is the line number and Y is the indentation level) for start_line, end_line and modified_data.",
                                                "- In case the modification content (modified_data) for a single range (start_line-end_line) is too long to include in one entry:",
                                                "    - Please split it across multiple answer entries.",
                                                "    - Each of these answer entries should maintain the same file_path, start_line, and end_line values",
                                                "    - Include modification_part representing the number of the current part in each entry to track the split.",
                                                #"    - Include modification_part information in each entry to track the split:",
                                                #"        - current: the number of the current part:",
                                                #"        - total: the total number of parts",
                                                "    - please remain ongoing_in_mode and ongoing flags true until all parts are delivered",
                                                #"- Do not propose multiple modifications for the same line.",
                                                #"- Do not split your modifications across multiple entries in the answer array when they target the exact same line range. For each unique (start_line, end_line) pair, there should be only ONE modification entry in your answer.",
                                                "- Set the ready_to_execute flag to True if this response marks the end of a coherent modification set and it's ready to be tested",
                                                #"- The 'Line X:' prefix is not part of the actual code - they're just line indicators. When providing modified_data, please use the correct indentation from the original code, ignoring the prefix.",
                                                f"- For file_path, write the relative path starting from {work_dir}.",
                                                "- Insert appropriate indentation in modified_data so that it can be executed correctly when copied and pasted into the original code's location from start_line to end_line.",
                                                "- \"modified_data\" must not contain any omissions and must strictly maintain the appropriate indentation, as it will be directly inserted and executed in the original code.",
                                                "- Ensure that \"modified_data\" follows the exact indentation level [Y] shown for each line in the original code.",
                                                "- If you want to only perform deletion without inserting into a specific location in the existing specified file path, set the value of 'is_deletion' to True.",
                                                #"- If you want to overwrite the entire file rather than just modifying the specified line range, set the value of 'overwrite_all' to True.",
                                                "- Set the value of \"no_simplification\" to True if the functionality intended before modification exists completely without any omission and simplification. Set it to False otherwise.",
                                                "- If the target file for editing is a JSON file, set the \"is_JSON\" flag to True and insert the modified JSON data into \"modified_data\"",
                                                #""
                                                #"- In modification, do not use unsafe or raw pointers. Instead, please use safe Rust types and operations including custom structs, enums, Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.", #"- NEVER use unsafe or raw pointers. Only use safe Rust types and operations: Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.", "- In modifications, please avoid using unsafe, and use the Rust standard library or crates to achieve equivalent functionality in a safe manner.",
                                                "- In modification, do not use unsafe, raw pointers, or manual memory management as much as possible.",
                                                "    - EXCEPTION: Permit minimal unsafe blocks strictly limited to the following two categories.",
                                                "               - Stub implementation of the FFI boundary functions (specified in ## FFI boundary functions below): These C functions are being replaced by Rust. These are declared as extern C fn with #[unsafe(no_mangle)] so that C code can call the Rust replacement. Stub implementation of the FFI boundary functions MUST remain unchanged until you are explicitly instructed to replace them with the actual implementation. Do NOT implement the actual logic of it.", # Note that even inside FFI boundary functions, extract logic into safe Rust helper functions.",
                                                #"               - Global variables: C global variables shared across the boundary, accessed through unsafe extern C static declarations with getter and setter functions.",
                                                "    - For everything else, write in safe Rust without calling C functions through FFI.",
                                                "- When representing backslashes as byte literals, escape the backslash twice in the source code, and also escape it again in the byte literal, resulting in four backslashes (double backslashes).",
                                                "- When representing backslashes as character literals, escape the backslash once in the source code and again in the character literal, resulting in two backslashes.",
                                                "- Please ensure that all function names have the prefix \"rust_\".",
                                            ])
                            

                            # "- Because we have the goal to have memory-safe Rust code to enhance security, please avoid using unsafe, and use the Rust standard library or crates to achieve equivalent functionality in a safe manner.",
                            # #"- When translating C raw pointers to Rust, use appropriate safe representations such as references, slices, Option, Box, Rc, Arc, etc.",
                            # #f"- The content of the corrected code in 'modified_data' must not contain any omissions because we execute the provided code directly. Do NOT use comments like \"// Implementation omitted for brevity\" or similar.",
                            # #"- Additionally, 'modified_data' must ensure proper indentation as the code will be copied and pasted directly between start_line and end_line.", # Ensure the code can be compiled.
                            # #"- If an error occurs when representing a backslash as a byte literal, escape the backslash in the source code and also in the byte literal by using four backslashes (double backslashes).",
                            # "- If an error occurs when representing a backslash as a character literal, escape the backslash in the source code and also in the character literal by using two backslashes.",
                            
                            prompt.extend(["\n## FFI boundary functions:"])
                            for func in functions:
                                prompt.append(f"   - {func['name']} (from {func['file_path']}, line {func['start_line']})") # lines {func['start_line']}-{func['end_line']})")
                            

                            prompt.extend(["\n## Response format", "In summary, please respond in the following JSON format:"]) 
                            prompt.extend([modify_template])

                            for seq_path in sequences:
                                # Sometimes gets the path wrong here
                                # if not os.path.exists(seq_path):
                                #     seq_code = ""
                                # else:
                                seq_code = get_lined_code(seq_path, work_dir)  #read_file(seq_path)
                                prompt.extend(["", f"## Code in {seq_path}:", seq_code])

                            
                            prompt.extend(["", f"## Error:", error])

                            sum_modified_list = []
                            mod_count = 1
                            while(1):
                                child_rsp_json = ask_llm(prompt, "continue", llm_interface)

                                # Need to update ongoing here
                                if 'ongoing' in child_rsp_json:
                                    ongoing_flag = child_rsp_json['ongoing']

                                if 'ongoing_in_mode' in child_rsp_json:
                                    ongoing_in_mode_flag = child_rsp_json['ongoing_in_mode']
                                
                                print(f"ongoing_flag at location 2 is {ongoing_flag}")
                                print(f"ongoing_in_mode_flag at location 2 is {ongoing_in_mode_flag}")

                                if 'mode' in child_rsp_json:
                                    mode = child_rsp_json['mode']

                                    if mode == 'modify_data':
                                        if 'answer' in child_rsp_json:
                                            modified_list = child_rsp_json['answer'] # It might be okay to put individually converted ones here
                                            if not isinstance(modified_list, list):
                                                modified_list = [modified_list]
                                            sum_modified_list.extend(modified_list)

                                    else: # Is this part even needed
                                        rsp_json = child_rsp_json
                                
                                ready_to_execute = False
                                if 'ready_to_execute' in child_rsp_json:
                                    ready_to_execute = child_rsp_json['ready_to_execute']
                                
                                if ready_to_execute is True or ongoing_in_mode_flag is False:
                                    break

                                # Might be better not to accept ongoing here
                                """
                                if ongoing_in_mode_flag is False:
                                    print("Breaking in child modifying loop") 
                                    break
                                else:
                                """
                                #if mod_count > 3:
                                #    break

                                prompt = []

                                if FFI_STRATEGY == "preserve":
                                    prompt.extend([f"Continue writing the actual modifications for the file {seq_string} in modify_data mode.",
                                            "",
                                            "## Response rules",
                                            "- Please insert the filename, start line, and end line of the section to be deleted into the \"file_path\", \"start_line\", and \"end_line\" keys in the JSON data.",
                                            "- Then, insert the new content that should be inserted at that [start_line] into the value of the \"modified_data\" key.",
                                            "- Detailed modification process is as follows. Please carefully write start_line, end_line and modified_data considering the process:",
                                            "    1. All code in the specified range (from [start_line] to [end_line]) will be completely deleted.",
                                            "    2. The content you provide in \"modified_data\" will be inserted at [start_line].",
                                            "    3. All code from [end_line + 1] onwards will remain unchanged and be appended after your modified_data.",
                                            "- Please use the exact line numbers and indentation levels shown on the left side of the code (Line X [Y], where X is the line number and Y is the indentation level) for start_line, end_line and modified_data.",
                                            "- In case the modification content (modified_data) for a single range (start_line-end_line) is too long to include in one entry:",
                                            "    - Please split it across multiple answer entries.",
                                            "    - Each of these answer entries should maintain the same file_path, start_line, and end_line values",
                                            "    - Include modification_part representing the number of the current part in each entry to track the split.",
                                            #"    - Include modification_part information in each entry to track the split:",
                                            #"        - current: the number of the current part:",
                                            #"        - total: the total number of parts",
                                            "    - please remain ongoing_in_mode and ongoing flags true until all parts are delivered",
                                            #"- Do not propose multiple modifications for the same line.",
                                            #"- Do not split your modifications across multiple entries in the answer array when they target the exact same line range. For each unique (start_line, end_line) pair, there should be only ONE modification entry in your answer.",
                                            "- Set the ready_to_execute flag to True if this response marks the end of a coherent modification set and it's ready to be tested",
                                            #"- The 'Line X:' prefix is not part of the actual code - they're just line indicators. When providing modified_data, please use the correct indentation from the original code, ignoring the prefix.",
                                            f"- For file_path, write the relative path starting from {work_dir}.",
                                            "- Insert appropriate indentation in modified_data so that it can be executed correctly when copied and pasted into the original code's location from start_line to end_line.",
                                            "- \"modified_data\" must not contain any omissions and must strictly maintain the appropriate indentation, as it will be directly inserted and executed in the original code.",
                                            "- Ensure that \"modified_data\" follows the exact indentation level [Y] shown for each line in the original code.",
                                            "- If you want to only perform deletion without inserting into a specific location in the existing specified file path, set the value of 'is_deletion' to True.",
                                            #"- If you want to overwrite the entire file rather than just modifying the specified line range, set the value of 'overwrite_all' to True.",
                                            "- Set the value of \"no_simplification\" to True if the functionality intended before modification exists completely without any omission and simplification. Set it to False otherwise.",
                                            "- If the target file for editing is a JSON file, set the \"is_JSON\" flag to True and insert the modified JSON data into \"modified_data\"",
                                            #"",
                                            #"- In modification, do not use unsafe or raw pointers. Instead, please use safe Rust types and operations including custom structs, enums, Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.", #"- NEVER use unsafe or raw pointers. Only use safe Rust types and operations: Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.","- In modifications, please avoid using unsafe, and use the Rust standard library or crates to achieve equivalent functionality in a safe manner.",
                                            "- In modification, do not use unsafe, raw pointers, or manual memory management as much as possible.",
                                            "    - EXCEPTION: Permit minimal unsafe blocks strictly limited to the following two categories.",
                                            "                  1. Stub implementation of the FFI boundary functions (specified in ## FFI boundary functions below): These C functions are being replaced by Rust. These are declared as extern C fn with #[unsafe(no_mangle)] so that C code can call the Rust replacement. Stub implementation of the FFI boundary functions MUST remain unchanged until you are explicitly instructed to replace them with the actual implementation. Do NOT implement the actual logic of it.", # Note that even inside FFI boundary functions, extract logic into safe Rust helper functions.",
                                            "                  2. Global variables: C global variables shared across the boundary, accessed through unsafe extern C static declarations with getter and setter functions.",
                                            #"                  3. cfg attribute flags: Conditional compilation flags registered as cargo rustc cfg, used with #[cfg(has_FLAG_NAME)] attributes.",
                                            #"                  4. Independent constant macros: C macro constants generated by bindgen in bindings.rs, used directly by name without redefinition.",
                                            "    - For everything else, write in safe Rust without calling C functions through FFI.",
                                            "- When representing backslashes as byte literals, escape the backslash twice in the source code, and also escape it again in the byte literal, resulting in four backslashes (double backslashes).",
                                            "- When representing backslashes as character literals, escape the backslash once in the source code and again in the character literal, resulting in two backslashes.",
                                            "- After translating functions to Rust, please add the prefix \"rust_\" to all function names.",
                                            ])

                                else:
                                    prompt.extend([f"Continue writing the actual modifications for the file {seq_string} in modify_data mode.",
                                            "",
                                            "## Response rules",
                                            "- Please insert the filename, start line, and end line of the section to be deleted into the \"file_path\", \"start_line\", and \"end_line\" keys in the JSON data.",
                                            "- Then, insert the new content that should be inserted at that [start_line] into the value of the \"modified_data\" key.",
                                            "- Detailed modification process is as follows. Please carefully write start_line, end_line and modified_data considering the process:",
                                            "    1. All code in the specified range (from [start_line] to [end_line]) will be completely deleted.",
                                            "    2. The content you provide in \"modified_data\" will be inserted at [start_line].",
                                            "    3. All code from [end_line + 1] onwards will remain unchanged and be appended after your modified_data.",
                                            "- Please use the exact line numbers and indentation levels shown on the left side of the code (Line X [Y], where X is the line number and Y is the indentation level) for start_line, end_line and modified_data.",
                                            "- In case the modification content (modified_data) for a single range (start_line-end_line) is too long to include in one entry:",
                                            "    - Please split it across multiple answer entries.",
                                            "    - Each of these answer entries should maintain the same file_path, start_line, and end_line values",
                                            "    - Include modification_part representing the number of the current part in each entry to track the split.",
                                            #"    - Include modification_part information in each entry to track the split:",
                                            #"        - current: the number of the current part:",
                                            #"        - total: the total number of parts",
                                            "    - please remain ongoing_in_mode and ongoing flags true until all parts are delivered",
                                            #"- Do not propose multiple modifications for the same line.",
                                            #"- Do not split your modifications across multiple entries in the answer array when they target the exact same line range. For each unique (start_line, end_line) pair, there should be only ONE modification entry in your answer.",
                                            "- Set the ready_to_execute flag to True if this response marks the end of a coherent modification set and it's ready to be tested",
                                            #"- The 'Line X:' prefix is not part of the actual code - they're just line indicators. When providing modified_data, please use the correct indentation from the original code, ignoring the prefix.",
                                            f"- For file_path, write the relative path starting from {work_dir}.",
                                            "- Insert appropriate indentation in modified_data so that it can be executed correctly when copied and pasted into the original code's location from start_line to end_line.",
                                            "- \"modified_data\" must not contain any omissions and must strictly maintain the appropriate indentation, as it will be directly inserted and executed in the original code.",
                                            "- Ensure that \"modified_data\" follows the exact indentation level [Y] shown for each line in the original code.",
                                            "- If you want to only perform deletion without inserting into a specific location in the existing specified file path, set the value of 'is_deletion' to True.",
                                            #"- If you want to overwrite the entire file rather than just modifying the specified line range, set the value of 'overwrite_all' to True.",
                                            "- Set the value of \"no_simplification\" to True if the functionality intended before modification exists completely without any omission and simplification. Set it to False otherwise.",
                                            "- If the target file for editing is a JSON file, set the \"is_JSON\" flag to True and insert the modified JSON data into \"modified_data\"",
                                            #"",
                                            #"- In modification, do not use unsafe or raw pointers. Instead, please use safe Rust types and operations including custom structs, enums, Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.", #"- NEVER use unsafe or raw pointers. Only use safe Rust types and operations: Vec, Box, Arc, Rc, String, HashMap, and other standard library collections that provide automatic memory management.","- In modifications, please avoid using unsafe, and use the Rust standard library or crates to achieve equivalent functionality in a safe manner.",
                                            "- In modification, do not use unsafe, raw pointers, or manual memory management as much as possible.",
                                            "    - EXCEPTION: Permit minimal unsafe blocks strictly limited to the following two categories.",
                                            "               - Stub implementation of the FFI boundary functions (specified in ## FFI boundary functions below): These C functions are being replaced by Rust. These are declared as extern C fn with #[unsafe(no_mangle)] so that C code can call the Rust replacement. Stub implementation of the FFI boundary functions MUST remain unchanged until you are explicitly instructed to replace them with the actual implementation. Do NOT implement the actual logic of it.", # Note that even inside FFI boundary functions, extract logic into safe Rust helper functions.",
                                            #"               - Global variables: C global variables shared across the boundary, accessed through unsafe extern C static declarations with getter and setter functions.",
                                            "    - For everything else, write in safe Rust without calling C functions through FFI.",
                                            "- When representing backslashes as byte literals, escape the backslash twice in the source code, and also escape it again in the byte literal, resulting in four backslashes (double backslashes).",
                                            "- When representing backslashes as character literals, escape the backslash once in the source code and again in the character literal, resulting in two backslashes.",
                                            "- After translating functions to Rust, please add the prefix \"rust_\" to all function names.",
                                            ])


                                prompt.extend(["\n## FFI boundary functions:"])
                                for func in functions:
                                    prompt.append(f"   - {func['name']} (from {func['file_path']}, line {func['start_line']})") # lines {func['start_line']}-{func['end_line']})")
                                
                                                            
                                prompt.extend(["\n## Response format", "In summary, please respond in the following JSON format:"]) 
                                prompt.extend([modify_template])

                                """
                                #  Let's remove this for now
                                for seq_path in sequences:
                                    # Sometimes gets the path wrong here
                                    # if not os.path.exists(seq_path):
                                    #     seq_code = ""
                                    # else:
                                    seq_code = get_lined_code(seq_path, work_dir)  #read_file(seq_path)
                                    prompt.extend(["", f"## Code in {seq_path}:", seq_code])
                                """
                                    
                                mod_count += 1
                            

                            prompt = []
                            ongoing_in_mode_flag = False # added
                            ongoing_flag = False # added

                # This may continue from the fix child mode section, so it must be placed below modify_data with an if statement
                if mode == 'read_data':
                    if 'answer' in rsp_json:
                        code = rsp_json['answer']
                        append_file(execute_path, code)

                    if 'target_files' in rsp_json and rsp_json['target_files'] is not None: #if 'target_files' in rsp_json:
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
            
            if repair_target == "build":
                if repair_count == 1:
                    prompt = [f"Please continue the JSON data response with the converted Rust code."]
                    #prompt = [f"Please continue the JSON data response for creating test cases for the {rust_path} file."]
                else:
                    prompt = [f"Please continue the JSON data response for resolving the compile errors."]

            else: # Just roughly grouping these together here
                prompt = [f"Please follow the response rules and format, and continue the JSON data response."]

            # This might not be needed
            prompt.extend(["## Response rules",  
                        f"- To avoid hitting the token limit, keep the JSON data included in one response within {output_max} tokens.",
                        "- If the JSON data for a single mode response is likely to exceed the token limit, split the response into multiple parts.",
                        "- If the JSON data is the last part, set the `ongoing_in_mode` key to `False`. If there are remaining JSON data parts, set the `ongoing_in_mode` key to `True`.",
                        "- Each response should always be in a single mode (`read_data`, `modify_data`, `execute_command`), and `ongoing_in_mode` should only be used when further interaction is needed within that mode.",
                        "- If you want to switch modes, end the current mode by setting ongoing_in_mode to false",
                        "- The `modified_data` content in `modify_data` mode must be directly executable without any omissions.",                        
                        ])
                
            rsp_json = ask_llm(prompt, "continue", llm_interface) 


        ######################## Proceed per file ########################

        print(f"Running program for the mode: {mode}")
        if mode == 'modify_data':
            print(f"In mode: {mode}")
            print(f"Modifying at repair_count: {repair_count}")
            # Cannot include modifications for the same start_line and end_line. For example, in the case of a split response with start_line = 1, end_line = 600, start_line and end_line remain the same throughout
            part_editied_files = reflect_line_modification(sum_modified_list, work_dir, database_dir)
            modified_c_keys = update_modified_keys(modified_c_keys, meta_dir, rust_c_map, part_editied_files)
            editied_files.extend(part_editied_files)

            #if not reflect_success:
            #    return repair_count, error
        
        elif mode == 'read_data':
            print(f"In mode: {mode}")
            #output = run_read_script(execute_path, 50, True, None, "both")
            read_prompt = ["The content obtained in read_data mode is as follows.", ""] 
            
            for see_path in sum_target_list:
                file_code = get_lined_code(see_path, work_dir)
                read_prompt.extend([f"- Content of the file {see_path}:"])  
                if len(file_code) == 0:
                    file_code = f"Line 1 [0]: [This {see_path} file is currently empty and contains no content. *** STOP *** Do not use read_data mode anymore.]"
                read_prompt.extend([f'{file_code}\n'])

            for see_item in sum_slice_list:
                file_code = get_lined_specific_code(database_dir, see_item['file_path'], see_item['start_line'], see_item['end_line'])
                read_prompt.extend([f"- Content of {see_item['start_line']} - {see_item['end_line']} lines in the file {see_item['file_path']}:"]) 
                read_prompt.extend([f'{file_code}\n'])

            print("End of rsp_json")
        
        elif mode == 'execute_command':
            print(f"In mode: {mode}")
            execute_error, execute_out, repair_count = run_script(execute_path, 50, True, None, "both", None, repair_count, None, None, mode)
            
            """
            if process_type == "explore":
                is_covered = get_is_covered(entry, cov_detail_path, target_dir, cov_dir) 
            """
            #rsp_json = ask_llm(prompt, "continue", interface)

        repair_count += 1
        #modified_file_list.extend(sum_modified_list)

    iteration_dict[rust_path] = repair_count
    judge_dict[rust_path] = judge_count
    update_judge_dict(rust_path, judge_count, f"{database_dir}/judge.json")

    if repair_target == "build" or repair_target == "compile":
        seen_files = set()
        for edite_file in editied_files:
            if edite_file not in seen_files:
                seen_files.add(edite_file)

        return modified_c_keys
    
    elif repair_target == "ask_generates" or repair_target == "ask_correspondence":
        data = read_json(answer_path)
        write_json(answer_path, data)

    
    return modified_c_keys


def concatanate_rust_files(rust_path_list):
    concatenated_content = ""  # String to hold the concatenated content
    for file_path in rust_path_list:
        try:
            with open(file_path, 'r', encoding='utf-8') as file:
                content = file.read()
                concatenated_content += content + "\n"  # Insert a newline between files
        except FileNotFoundError:
            print(f"Error: File not found - {file_path}")
        except IOError:
            print(f"Error: IO error while reading file - {file_path}")

    return concatenated_content


def show_rust_items(prompt, file_path, json_data): 
    for item in json_data:
        if item['end_line'] is None:
            continue
        item_code = read_specific_lines(file_path, item['start_line'], item['end_line'])
        prompt.append(f"    {item_code}")

    return prompt


def show_rust_codes(prompt, json_data):
    for item in json_data:
        print(f"item in show_rust_codes: {item}")
        item_code = item['rust_code']
        prompt.append(f"{item_code}") # Better to have no whitespace since the data is copied as-is
    
    return prompt


# This needs to look at all_macro_path
def refer_macro_define(all_macro_path, type_signal, defined_flag): # file_path, meta_dir, 
    found_macros = []
    #print(all_macro_path)
    macro_data = read_json(all_macro_path)

    if macro_data is not None:
        for macro, entries in macro_data.items():
            first_json = entries[0]
            #print(first_json)
            if 'macro_det' in first_json and first_json['macro_det'] == type_signal:
                found_macros.append(first_json)
        
    return found_macros


def get_rust_code(source_path, target_item):
    target_rust_code = None

    meta_data, meta_path = obtain_metadata(source_path, meta_dir, False, None, "def")
    for item in meta_data:
        if item['element_id'] == target_item['element_id']:
            if 'rust_code' in item:
                target_rust_code = item['rust_code']['content']
            break

        if 'components' in item:
            for com in item['components']:
                if com['element_id'] == target_item['element_id']:
                    if 'rust_code' in com:
                        target_rust_code = com['rust_code']['content']
                    break
    
    return target_rust_code


def get_macro_define(macro_path, all_macro_path, item):
    rust_code = None

    macro_json = read_json(macro_path)
    if macro_json is not None:
        found = False
        for macro_name, entries in macro_json.items():
            if macro_name == item['name']:
                found = True
                if 'rust_macro_define' in entries[0]:
                    rust_code = entries[0]['rust_macro_define']
                
                break
        if found:
            write_json(macro_path, macro_json)


    macro_json = read_json(all_macro_path)
    if macro_json is not None:
        found = False
        for macro_name, entries in macro_json.items():
            if macro_name == item['name']:
                found = True
                if 'rust_macro_define' in entries[0]:
                    rust_code = entries[0]['rust_macro_define']
        
        if found:
            write_json(all_macro_path, macro_json)

    return rust_code


def get_target_flag(item_data):
    target_flag = False

    if 'is_target' in item_data:
        target_flag = item_data['is_target']

        print(target_flag)
    return target_flag


def get_independent_flag(item_data):
    independent_flag = False

    if 'is_independent' in item_data:
        independent_flag = item_data['is_independent']

    return independent_flag


def get_const_flag(item_data):
    const_flag = False

    if 'is_const' in item_data:
        const_flag = item_data['is_const']

    return const_flag



def get_global_flag(item_data):
    global_flag = False

    if item_data['kind'] == 'global_var':
        global_flag = True

    return global_flag


def get_ifdef_flag(item_data):
    ifdef_flag = False

    if 'ifdef_statement' in item_data:
        ifdef_flag = item_data['ifdef_statement']
    return ifdef_flag



def get_rust_items(item_data):
    rust_code = None

    if 'rust_code' in item_data and 'content' in item_data['rust_code']:
        rust_code = item_data['rust_code']['content']

    """
    if 'rust_code' in item_data: # and 'content' in item_data['rust_code']:
        rust_code = item_data['rust_code'] #['content']
    """
    return rust_code



def collect_dependencies(cashed, c_items, meta_path, meta_data, dep_json_path, is_program_path,
                         div_meta_dir, original_dir, build_path, conv_type, 
                         i_at_least_found, independent_macros, if_at_least_found, ifdefs, r_at_least_found, rust_refs, 
                         g_at_least_found, global_vars, t_at_least_found, targets, seen, target,
                         g_used, f_used, i_used, user_id):
    
    prompt = []
    repair_prompt = []

    rust_flow = []
    rust_flow_dict = {}

    program_files = set(read_json(is_program_path))
    for c_item in c_items:
        target_name = c_item['name']
        file_path = c_item['file_path']
        start_line = c_item['start_line']
        end_line = c_item['end_line']

        key_name = f"{target_name}:{file_path}:{start_line}"
        print(key_name)
        
        ###############################
        #### Uses
        ###############################

        if meta_data is None:
            raise ValueError(f"Failed to find {key_name} at {meta_path}")

        if key_name not in meta_data:
            raise ValueError(f"Failed to find {key_name} at {meta_path}")
            
        if 'uses' in meta_data[key_name]:
            uses_list = meta_data[key_name]['uses']
        
            for use_item in uses_list:
                if 'file_path' in use_item:
                    use_file_path = use_item['file_path']
                    use_start_line = use_item['start_line']

                elif 'definition' in use_item:
                    use_definition = use_item['definition']
                    use_file_path, use_start_line, _col = parse_def_loc(use_definition)
    
                else:
                    raise ValueError("Must have definition or file_path key.")
                
                if use_start_line is None: # in case of undefined
                    continue
                use_start_line = int(use_start_line)

                c_use_file_path = use_file_path.replace(f"trans_c_{user_id}", f"workspace_{user_id}_{target}")
                if is_system_file(c_use_file_path, program_files):
                    continue

                # if original_dir not in use_file_path: # Not filtered out in the uses section # Needs checking
                #     continue

                use_name = use_item['name']
                use_key_name = f"{use_name}:{use_file_path}:{use_start_line}"

                use_meta_path = obtain_metadata(use_file_path, div_meta_dir, False, True, "def")

                if use_meta_path not in cashed:
                    use_meta_data = obtain_metadata(use_file_path, div_meta_dir,  False, False, "def")
                    cashed[use_meta_path] = {}
                    cashed[use_meta_path] = use_meta_data
                else:
                    use_meta_data = cashed[use_meta_path]
                
                # if meta_data is None: # Better to double-check this # This case exists: File not found: div_metadata_{user_id}/pp-patterns//usr/include/x86_64-linux-gnu/bits/types/struct_FILE_h.json
                #     continue

                ##### rust code (originated in Rust)
                if use_key_name not in use_meta_data: # In the case of local_var
                    continue

                rust_code = get_rust_items(use_meta_data[use_key_name])

                category = None
                if use_meta_data[use_key_name]['kind'] == "global_var":
                    #g_used = True
                    category = "global variable"
                
                if "macro" in use_meta_data[use_key_name]['kind']:
                    if 'is_const' in use_meta_data[use_key_name] and use_meta_data[use_key_name]['is_const'] is True and 'is_independent' in use_meta_data[use_key_name] and use_meta_data[use_key_name]['is_independent']:
                        #i_used = True
                        category = "independent const macros"
                
                if use_meta_data[use_key_name]['kind'] == "directive":
                    #f_used = True
                    category = "cfg attribute"

                if rust_code is not None:
                    added_name = target_name
                    if target_name == "top_level_use" and 'components' in meta_data[key_name]:
                        added_name = ""
                        com_list = meta_data[key_name]['components']
                        i = 0
                        for com_item in com_list:
                            if i == 0:
                                added_name = f"{com_item['name']}"
                            else:
                                added_name += f", {com_item['name']}"
                            i += 1

                    if added_name not in rust_refs:
                        rust_refs[added_name] = {} # rust_refs[target_name].add(rust_code) # added
                    if added_name not in seen:
                        rust_refs[added_name] = {
                            "name" : use_name,
                            "rust_code" : rust_code,
                            "category" : category
                        }
                    r_at_least_found = True

                    # Only set the flag if actually added to rust_refs
                    if category == "global variable":
                        g_used = True
                    elif category == "independent const macros":
                        i_used = True
                    elif category == "cfg attribute":
                        f_used = True
                                

        ###############################
        #### Definition itself
        ###############################
        ##### whether the function is target or not
        target_flag = get_target_flag(meta_data[key_name])

        if target_flag is True: # and is_const is True:
            if c_item['name'] not in targets:
                targets[c_item['name']] = set()
            targets[c_item['name']].add(target_name) 
            t_at_least_found = True
            #continue

        ##### independent constant macros (originated in C and Rust/build.rs)
        independent_flag = get_independent_flag(meta_data[key_name])
        is_const = get_const_flag(meta_data[key_name])

        if independent_flag is True: # and is_const is True:
            if c_item['name'] not in independent_macros:
                independent_macros[c_item['name']] = set()
            independent_macros[c_item['name']].add(target_name) 
            i_at_least_found = True
            continue

        ##### global variables (originated in C) # Should not need special handling in build.rs
        global_flag = get_global_flag(meta_data[key_name])

        if global_flag is True:
            if c_item['name'] not in global_vars:
                global_vars[c_item['name']] = set()
            global_vars[c_item['name']].add(target_name) 
            g_at_least_found = True
            continue

        ##### cfg states (originated in Rust/build.rs)
        ifdef_key_name = f"IFDEF:{file_path}:{start_line}"
        if ifdef_key_name in meta_data:                            
            macro_name = meta_data[ifdef_key_name]['name']  # Since it's IFDEF, use name instead of macro_name  
            ifdefs.add(macro_name) 
            if_at_least_found = True

        if_key_name = f"IF:{file_path}:{start_line}"
        if if_key_name in meta_data:
            if "condition" in meta_data[if_key_name]:     
                if "defined" in meta_data[if_key_name]["condition"]:
                    for macro_item in meta_data[if_key_name]["macros"]:
                        macro_name = macro_item['name'] 
                        ifdefs.add(macro_name) 
                        if_at_least_found = True

    # repair_prompt = prompt
    dependencies = {
        "prompt": prompt,
        "cashed": cashed,
        "t_at_least_found": t_at_least_found,
        "targets": targets,
        "i_at_least_found": i_at_least_found,
        "independent_macros": independent_macros,
        # "d_at_least_found": d_at_least_found,
        # "dependent_macros": dependent_macros,
        "if_at_least_found": if_at_least_found,
        "ifdefs": ifdefs,
        "r_at_least_found": r_at_least_found,
        "rust_refs": rust_refs,
        "g_at_least_found": g_at_least_found,
        "global_vars": global_vars,
        "g_used" : g_used,
        "f_used" : f_used,
        "i_used" : i_used,
    }

    return dependencies 


def get_rust_function_name(name_key, c_rust_map):
    return c_rust_map[name_key]


def collect_rust_dependencies(cashed, c_item, dep_json_path, meta_dir, build_path, conv_type, c_rust_map):
    
    c_name = c_item['name']
    c_path = c_item['file_path']
    start_line = c_item['start_line']
    end_line = c_item['end_line']

    key_name = f"{c_name}:{c_path}:{start_line}"

    prompt = []
    repair_prompt = []

    # Get the originally recorded called functions for the C side
    rust_flow = []
    rust_flow_dict = {}
    rust_refs = {}

    meta_path = obtain_metadata(c_path, meta_dir, False, True, "def")
    if meta_path not in cashed:
        c_meta_data = obtain_metadata(c_path, meta_dir, False, False, "def")
        cashed[meta_path] = {}
        cashed[meta_path] = c_meta_data
    else:
        c_meta_data = cashed[meta_path]

    element_item = c_meta_data[key_name]
    #for element_item in c_meta_data:
    #if element_item['category'] == 'function':  # Skipped this but is that okay: if element_item['block_type'] == 'function' and element_item['category'] == 'function':
    function_name = element_item['name']
    c_flow = element_item['uses']
    name_key = get_name_key(element_item)

    for item in c_flow:
        rust_code = get_rust_items(cased, item)

        if rust_code is not None:
            if name_key not in rust_refs: #function_name not in rust_refs:
                rust_refs[name_key] = []

            rust_refs[name_key].append(rust_code) # added

    at_least_found = False
    keys_to_delete = []  # List to keep track of keys to be deleted

    for name_key, func_list in rust_refs.items():
        has_no_func_def = False
        if any(func_list):
            for func_dict in func_list:
                for func, file_path in func_dict.items():
                    if func is not None:
                        at_least_found = True
                        has_no_func_def = True
        if not has_no_func_def:
            rust_func_name = get_rust_function_name(name_key, c_rust_map)
            keys_to_delete.append(rust_func_name)  # Mark the key for deletion

    # Delete the keys after the iteration
    for key in keys_to_delete:
        del rust_refs[key]

    if at_least_found:
        prompt.extend([
            "- Items being called:", # Referencing (using) functions
            #"  - The functions being called from each function have already been translated into Rust as shown below."
            "    - Please use the following items that have already been translated into Rust."
        ])

    for func_name, func_list in rust_refs.items():
        if any(func_list):
            prompt.extend([f"  {func_name} (caller):"])
            func_set = set() # Avoid duplicates
            for func in func_list:
                prompt.append(f"    {func}")  #prompt.append(f"    {func}  (defined in {file_path})")
                repair_prompt.append(f"    {func}")  #repair_prompt.append(f"    {func}  (defined in {file_path})")
                func_set.add(func) 
                """
                for func, file_path in func_dict.items():  # Extract keys and values from func_dict
                    if file_path not in [lib_path, build_path, cargo_path]:
                        mod_name = generate_rust_import(file_path)
                        if func is not None and func not in func_set:
                            prompt.append(f"    {func}  (defined in {file_path} and can be imported using {mod_name})")  #prompt.append(f"    {func}  (defined in {file_path})")
                            repair_prompt.append(f"    {func}  (defined in {file_path} and can be imported using {mod_name})")  #repair_prompt.append(f"    {func}  (defined in {file_path})")
                            func_set.add(func)   
                """    
    repair_prompt = prompt
    return prompt, repair_prompt, cashed



#########################################################
# conversion main functions
#########################################################


def unify_divisions(rust_path, parts_rust_paths):
    print("Unifying divisions")
    # No need to delete rust_path
    # The order matters here — process from the rust_path that was converted first
    try:
        with open(rust_path, 'w') as rust_file:
            for div_path in parts_rust_paths:
                with open(div_path, 'r') as part_file:
                    rust_file.write(part_file.read())
                    rust_file.write('\n')  # Add a newline after each file
        print(f"Successfully unified the contents into {rust_path}.")
    except Exception as e:
        print(f"An error occurred in unify_divions: {e}")
        

def insert_rust_source(dep_json_path):
    dep_json = read_json(dep_json_path)
    for item in dep_json:
        c_source = item['source']
        rust_source = obtain_rust_path(c_source)
        item['rust_source'] = rust_source

    write_json(dep_json_path, dep_json)


def insert_unimplemented(c_path, answer_path):
    answer_json = read_json(answer_path)
    for ans in answer_json:
        file_path = ans['c_file_path']
        if not os.path.exists(file_path):
            file_path = find_matching_path(work_dir, file_path)

        name = ans['c_name']
        rust_name = ans['name']
        rust_path = ans['rust_file_path']
        rust_start = ans['rust_start_line']
        rust_end = ans['rust_end_line']
        meta_data, meta_path = obtain_metadata(file_path, meta_dir, False, None, "def")
        if meta_data is None:
            continue
        for item in meta_data:
            if name == item['name']:
                item['source_memo'] = f"// The {name} is unimplemented in {rust_path} at line {rust_start} - {rust_end} as {rust_name} now. Please write the exact definition in the function and complete the implementation based on the original C functionality."
        
        write_json(meta_path, meta_data)



def translate_unit(one_unit, work_dir, raw_dir, target_dir, database_dir, original_dir, 
                   target_path, chat_dir, token_path, time_path, progress_queue, log_dir, max_iterations,
                   meta_dir, div_meta_dir, dep_json_path, rust_output_dir, 
                   build_path, lib_path, build_config_path, run_test_path, run_all_path, error, return_path, exp_data,
                   trial_id, rust_build_path, llm_interface, rust_edition, 
                   c_rust_path, rust_c_path, is_program_path, target, repair_only
                   ):
    
    ###############################
    ###### Translation
    ###############################
    
    # May need the parent path first
    rust_path = lib_path
    exp_data = {}
    repair_count = 0
    execute_path = f"{work_dir}/execute.sh"
    explore_time = 0
    notes = []
    
    if repair_only is False:

        interface = TransConfig(
            rust_c_path=rust_c_path,
            c_rust_path=c_rust_path,
            target_path=target_path,
            raw_dir=raw_dir,
            rust_output_dir=rust_output_dir,
            select=False,
            target_dir=target_dir,
            work_dir=work_dir,
            database_dir=database_dir,
            cov_target="function",
            original_target_dir=original_dir,
            build_path=build_path,
            rust_build_path=rust_build_path,
            run_test_path=run_test_path,
            run_all_path=run_all_path,
            execute_path=execute_path,
            meta_dir=meta_dir,
            div_meta_dir=div_meta_dir,
            dep_json_path=dep_json_path,
            exp_data=exp_data,
            llm_interface=llm_interface,
            chat_dir=chat_dir,
            repair_count=repair_count,
            rust_edition=rust_edition,
            token_path=token_path,
            time_path=time_path,
            explore_time=explore_time,
            notes=notes,
            progress_queue=progress_queue,
            log_dir=log_dir,
            max_iterations=max_iterations,
            is_program_path=is_program_path,
            target=target,
            user_id=user_id,
        )

        if FFI_STRATEGY == "preserve":
            translate_llm('divided_type', one_unit, rust_path, interface)

        else:
            translate_llm_minimize('divided_type', one_unit, rust_path, interface)

        # After conversion, generate blocks once: for build.rs, lib.rs
        # update_rust_block(rust_path, meta_dir, map_path)  #div_c_path, div_rust_path, meta_dir)


    ###############################
    ###### Iterative repair
    ###############################
    # compile & repair
    before_count = None

    repair_count = 2
    interface = {
        'convert_element': 'divided_type',
        'meta_dir': meta_dir,
        'dep_json_path': dep_json_path,
        'exp_data': exp_data,
        'repair_count': repair_count,
        'before_count': before_count,
        'target_dir': rust_output_dir,
        'raw_dir' : raw_dir,
        'rust_build_path' : rust_build_path
    }

    interface = TransConfig(
        rust_path=rust_path,
        rust_c_path=rust_c_path,
        c_rust_path=c_rust_path,
        raw_dir=raw_dir,
        rust_output_dir=rust_output_dir,
        select=False,
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
        rust_build_path=rust_build_path,
        run_test_path=run_test_path,
        run_all_path=run_all_path,
        meta_dir=meta_dir,
        div_meta_dir=div_meta_dir,
        dep_json_path=dep_json_path,
        exp_data=exp_data,
        repair_count=repair_count,
        rust_edition=rust_edition,
        execute_path=execute_path,
        explore_time=explore_time,
        notes=notes,
        progress_queue=progress_queue,
        log_dir=log_dir,
        max_iterations=max_iterations,
        target_path=target_path,
        target=target,
        user_id=user_id,
    )

    modified_c_keys = set()
    modified_c_keys = repair_execute('compile', interface)
    
    # Enumerate c_key elements that did not correspond to modified locations, and calculate unmodified_lines. Treat all other code lines as modified lines
    modified_rust_lines = get_modified_rust_lines(modified_c_keys, c_rust_path, meta_dir)


    c_mod_files = []
    rust_mod_files = []
    
    ###############################
    ## Correspondence mapping
    ###############################

    sum_answer_data = [] # {}

    ## Need handling for when repair was not needed in repair_execute
    modified_c_keys = merge_with_initial(one_unit, modified_c_keys)

    modified_lines = {}
    answer_path = f"{work_dir}/answer.json"

    grouped_c_keys = get_grouped_c_keys(modified_c_keys, 10)

    for c_key_json in grouped_c_keys:
        repair_count = 1
        interface = {
            'convert_element': 'divided_type',
            'meta_dir': meta_dir,
            'dep_json_path': dep_json_path,
            'exp_data': exp_data,
            'target_dir': rust_output_dir,
            'raw_dir' : raw_dir,
            'rust_build_path' : rust_build_path,
            'repair_count' : repair_count,
            'answer_path' : answer_path
        }
        
        interface = CorConfig(
            one_unit=one_unit,
            answer_path=answer_path,
            modified_lines=modified_lines,
            key_json=c_key_json,
            rust_path=rust_path,
            raw_dir=raw_dir,
            rust_output_dir=rust_output_dir,
            select=False,
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
            rust_build_path=rust_build_path,
            run_test_path=run_test_path,
            run_all_path=run_all_path,
            meta_dir=meta_dir,
            div_meta_dir=div_meta_dir,
            dep_json_path=dep_json_path,
            exp_data=exp_data,
            repair_count=repair_count,
            rust_edition=rust_edition,
            execute_path=execute_path,
            explore_time=explore_time,
            notes=notes,
            progress_queue=progress_queue,
            log_dir=log_dir,
            max_iterations=max_iterations,
            repair_max=REPAIR_MAX,
        )

        delete_file(answer_path)

        print(f"------ Start asking about the correspondence ------")
        ask_correspondence("ask_correspondence", interface)
        print(f"------ End of asking about the correspondence ------")

        # reverse answer_data
        mod_rust_path = rust_path
        reverse_tmp(answer_path, mod_rust_path, None, database_dir)

        # Want to merge the data from answer_path into sum_answer_data.
        answer_data = read_json(answer_path)
        if isinstance(answer_data, dict):
            if 'data' in answer_data:
                answer_data = answer_data['data']
            elif 'modified_data' in answer_data:
                answer_data = answer_data['modified_data']

        merge_list(sum_answer_data, answer_data) 


    ###############################
    ## Updating metadata
    ###############################

    write_json(f"{database_dir}/sum_answer.json", sum_answer_data)

    # update metadata
    update_metadata_with_rust(sum_answer_data, div_meta_dir, database_dir)




macro_template = f"""{{
    "answer" : {{
                    "(Macro variable name 1)" : "(The definition for Macro variable name 1 written in build.rs)",
                    "(Macro variable name 2)" : "(The definition for Macro variable name 2 written in build.rs)",
                    ...
                }}
    "ongoing" : true or false,
}}
"""

def update_order(return_path, initial_order):
    updated_order = []

    return_path_found = False
    for file_path in initial_order:
        if file_path == return_path:
            return_path_found = True
        if return_path_found:
            updated_order.append(file_path) # Pack everything from return_path onward into updated_order

    return updated_order


def get_random_order(order):
    random_order = []
    random_order = order[:]  # Create a copy of order
    random.shuffle(random_order)  # Shuffle the copied list

    return random_order


def set_moment_path(moment_path, average, log_file_path, target):
    moment_json = read_json(moment_path)
    if moment_json is None:
        moment_json = {}

    if target not in moment_json:
        moment_json[target] = {}
    
    if 'current_count' not in moment_json[target]:
        moment_json[target]['current_count'] = 1
    
    trial_id = "trial_" + str(moment_json[target]['current_count'])
    moment_json[target]['current_count'] += 1

    moment_json[target][trial_id] = {}

    if 'average' not in moment_json[target]:
        moment_json[target][trial_id]['average'] = None

    if 'paths' not in moment_json[target]:
        moment_json[target][trial_id]['paths'] = []

    if 'exp_logs' not in moment_json[target]:
        moment_json[target][trial_id]['exp_logs'] = []

    if WITH_CONDENSED:
        moment_json[target][trial_id]['conv_sum_prompt'] = True
    else:
        moment_json[target][trial_id]['conv_sum_prompt'] = False

    if REPAIR_MEMORIZED:
        moment_json[target][trial_id]['repair_history'] = True
    else:
        moment_json[target][trial_id]['repair_history'] = False

    if WITH_FILES:
        moment_json[target][trial_id]['conv_show_files'] = True
    else:
        moment_json[target][trial_id]['conv_show_files'] = False

    moment_json[target][trial_id]['average'] = average
    moment_json[target][trial_id]['log_file_path'] = log_file_path
    # moment_json[target].append(build_path)
    write_json(moment_path, moment_json)

    return trial_id



def set_convert_type(moment_path, target, convert_type, llm_choice):
    moment_json = read_json(moment_path)    
    trial_id = "trial_" + str(moment_json[target]['current_count']-1)

    moment_json[target][trial_id]['conv_type'] = convert_type

    if llm_choice == "claude":
        llm_model = claude_model
    elif llm_choice == "claude_azure":
        llm_model = "max-tokens-3-5-sonnet-2024-07-15"
    elif llm_choice == 'gpt':
        llm_model = gpt_model
    elif llm_choice == 'llama':
        llm_model = llama_model
    elif llm_choice == 'gemini':
        llm_model = gemini_model

    moment_json[target][trial_id]['llm_model'] = llm_model
    write_json(moment_path, moment_json)



def split_macro_type_data(input_path, unit_num, label):
    """
    Extract and split only macros whose macro_type is "label"
    
    Args:
        data (dict): Dictionary of macro definitions
        unit_num (int): Number of elements per file
    """
    data = read_json(input_path)

    # Extract only entries whose macro_type is "label"
    label_macros = {
        key: value for key, value in data.items() 
        if value.get("macro_type") == label
    }
    
    # Total number of label-type macros
    total_items = len(label_macros)
    
    # Calculate the number of splits
    num_parts = math.ceil(total_items / unit_num)
    
    # Convert macros to a list for easier splitting
    macro_items = list(label_macros.items())
    
    # Save the split data
    split_files = []
    for i in range(num_parts):
        start_idx = i * unit_num
        end_idx = min((i + 1) * unit_num, total_items)
        
        # Convert the split macros back to dictionary format
        part_data = dict(macro_items[start_idx:end_idx])
        
        # Save to file
        output_path = f'part_{i+1}.json'
        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(part_data, f, ensure_ascii=False, indent=4)
        
        split_files.append(output_path)
    
    return split_files, num_parts



def have_target_data(label, conds_status_path):
    conds_data = read_json(conds_status_path)
    found = False
    for macro, macro_info in conds_data.items():
        if macro_info['macro_type'] == label:
            print("We have at least one macro")
            found = True
            break
    
    return found


def initialize(translation_type, original_dir, rust_output_dir, work_dir, target_dir, raw_dir, database_dir, chat_dir, 
                map_path, logging_path, count_path, token_path, history_path, rust_c_path, c_rust_path, moment_path):

    if DEBUG_LLM: # This part is getting confusing. Since different files are needed at different times, delete as appropriate
        build_path = rust_output_dir + '/build.rs'
        delete_file(build_path)

    if not FROM_PART:
        delete_file(map_path)
    delete_file(f"{database_dir}/tmp_data.json")

    delete_directory(chat_dir)
    create_directory(chat_dir)

    delete_directory("exp")
    create_directory("exp")

    """
    data = read_json(count_path)
    if data is None:
        data = {}
    data["prompt_id"] = str(0).zfill(4)
    write_json(count_path, data)
    """

    delete_file(f"{database_dir}/judge.json")
    delete_file(token_path)

    write_json(rust_c_path, {}) # Initialization
    write_json(c_rust_path, {}) # Initialization

    if not DEBUG_LLM:
        delete_file(history_path)
        #delete_directory(exp_dir)
    
    # translation type
    global WITH_CONDENSED, WITH_FILES

    DIVIDED_TYPE = True
    PLAIN_TYPE = False
    WITH_CONDENSED = True
    WITH_FILES = False

    print("Initializing")
    if (MANUAL_FIRST or not MANUAL) or not DEBUG_LLM:
        print("Renewing the output directory.")
        if not FROM_PART:
            delete_directory(work_dir)
            delete_directory(rust_output_dir) # Better to delete only on the first time for MANUAL

            copy_directory(original_dir, work_dir)

            build_path = get_build_path(target_dir)
            run_script_wo_log(build_path, 1000, True, None, "both") # To align the path of compile_commands.json

    if not os.path.exists(moment_path):
        write_json(moment_path, {})


#########################################################
# Divide files
#########################################################

# Write results to a JSON file         
def insert_unit_lines(json_data, c_path, dep_json_path):
    dep_data = read_json(dep_json_path)
    for item in dep_data:
        if c_path == item['source']:
            item['div_parts'] = json_data

    write_json(dep_json_path, dep_data)


def approximate_parts(dep_json_path, meta_dir, adjusted_average):
    # Tentative notes
    # Assuming already sorted by 'parts_order' (order of divided module units)

    dep_data = read_json(dep_json_path)
    
    for dep in dep_data:
        file_path = dep['source']
        if 'div_parts' not in dep:
            continue

        div_json = dep['div_parts']
        meta_data = obtain_metadata(file_path, meta_dir, False, False, "def")
        parts_list = []
        new_parts = []

        count = 0  # Per single file_path
        for div in div_json:
            current_block = []

            b_start_line = div['start_line']
            b_end_line = div['end_line']
            
            # Find the nearest start line
            nearest_start_line = min(meta_data, key=lambda x: abs(x['start_line'] - b_start_line))['start_line']
            # Find the nearest end line
            nearest_end_line = min(meta_data, key=lambda x: abs(x['end_line'] - b_end_line))['end_line']
            
            # Adjust start line if it's within a block
            start_candidates = [item['start_line'] for item in meta_data if item['start_line'] <= b_start_line]
            if start_candidates:
                nearest_start_line = max(start_candidates)
            else:
                start_candidates = [item['start_line'] for item in meta_data if item['start_line'] >= b_start_line]
                if start_candidates:
                    nearest_start_line = min(start_candidates)
            
            # Adjust end line if it's within a block
            end_candidates = [item['end_line'] for item in meta_data if item['end_line'] >= b_end_line]
            if end_candidates:
                nearest_end_line = min(end_candidates)
            else:
                end_candidates = [item['end_line'] for item in meta_data if item['end_line'] <= b_end_line]
                if end_candidates:
                    nearest_end_line = max(end_candidates)
            
            base_path = file_path[:-2] + '_' + file_path[-1] + '/' + f'unit{count}.h'  # This path needs verification

            new_parts.append({'source': base_path, 'include' : [], 'start_line': nearest_start_line, 'end_line': nearest_end_line, 'parts_order': count})
            parts_list.append(base_path)

            count += 1

        # Adjust so that the next start_line follows right after each end_line
        for i in range(len(new_parts) - 1):
            new_parts[i]['end_line'] = new_parts[i + 1]['start_line'] - 1

        # Sort by start_line
        new_parts = sorted(new_parts, key=lambda x: x['start_line'])

        # For each entry, add to the list the sources of entries with a smaller start_line
        for part in new_parts:
            part['include'] = [elem['source'] for elem in new_parts if elem['start_line'] < part['start_line']]

        # Output results
        print(json.dumps(new_parts, indent=4, ensure_ascii=False))

        dep['div_parts'] = new_parts # Add to the 'div_parts' field of each original source
        dep['parts_list'] = parts_list

    write_json(dep_json_path, dep_data)



def extract_div_metadata(div_start_line, div_end_line, parent_meta_data):
    filtered_data = []
    for entry in parent_meta_data:
        if div_start_line <= entry['start_line'] <= div_end_line and div_start_line <= entry['end_line'] <= div_end_line:
            filtered_data.append(entry)
    
    return filtered_data



def get_source_element_id(source_path, target_item):
    target_element_id = None

    if source_path is None: # If this is macro
        return
    
    meta_data, meta_path = obtain_metadata(source_path, meta_dir, False, None, "def")
    
    if meta_data is None:
        return
    for item in meta_data:
        if item['category'] == target_item['category'] and item['name'] == target_item['name']:
            target_element_id = item['element_id']
            break

        if 'components' in item:
            for com in item['components']:
                if com['category'] == target_item['category'] and com['name'] == target_item['name']:
                    target_element_id = item['element_id']
                    break

    return target_element_id



def get_child_div(start_line, source_path):
    dep_json = read_json(dep_json_path)
    for item in dep_json:
        if 'div_parts' not in item:
            continue

        parent_c_path = item['source']
        if parent_c_path == source_path:
            for div in item['div_parts']:
                div_c_path = div['source']
                div_start_line = div['start_line']
                div_end_line = div['end_line']

                if div_start_line <= start_line and start_line <= div_end_line:
                    return div_c_path

    return None


def get_indirect_inclueds(file_path, dep_json_path):

    indirect_includes = []
    dep_data = read_json(dep_json_path)
    for item in dep_data:
        if item['source'] == file_path:
            indirect_includes = item['indirect_included']
            break

    return indirect_includes


def calculate_stats(data):
    if not data:
        return None, None

    # Calculate mean
    n = len(data)
    mean = sum(data) / n

    # Calculate standard deviation
    squared_diff_sum = sum((x - mean) ** 2 for x in data)
    variance = squared_diff_sum / n
    std_dev = math.sqrt(variance)

    return mean, std_dev



def insert_division(dep_json_path, target, line_path, average):
    dep_json = read_json(dep_json_path)

    distance_list = []
    
    for dep in dep_json:
        parent_path = dep['source']
        if 'div_parts' in dep:
            parts = dep['div_parts']
            for part in parts:
                code = read_specific_lines(parent_path, part['start_line'], part['end_line'])
                write_file("loc.c", code)
                distance = c_count_loc("loc.c")
                distance_list.append(distance)
                delete_file("loc.c")
                #distance = part['end_line'] - part['start_line']
        else:
            distance = c_count_loc(dep['source'])
            distance_list.append(distance)
        

    avg, std = calculate_stats(distance_list)

    line_data = read_json(line_path)
    if line_data is None:
        line_data = {}

    if target not in line_data:
        line_data[target] = {}
    
    if str(average) not in line_data[target]:
        line_data[target][str(average)] = {}
    
    line_data[target][str(average)]['div_list'] = distance_list
    line_data[target][str(average)]['line_avg'] = avg
    line_data[target][str(average)]['line_std'] = std
    
    write_json(line_path, line_data)


def create_units(dep_json_path):
    dep_json = read_json(dep_json_path)
    for item in dep_json:
        parent_path = item['source']

        if 'div_parts' in item:
            for div in item['div_parts']:
                div_c_path = div['source']
                div_start_line = div['start_line']
                div_end_line = div['end_line']
                c_code = read_specific_lines(parent_path, div_start_line, div_end_line)
                write_file(div_c_path, c_code)
        
        delete_file(parent_path)



def alphanumeric_sort_key(file_name): # Extract numeric parts to sort a mix of numbers and alphabets
    return [int(text) if text.isdigit() else text.lower() for text in re.split('([0-9]+)', file_name)]


def filter_block_path(block_path, previous_block_path):
    print(f"Filtered {block_path} (using {previous_block_path})")

    previous_blocks = set()
    with open(previous_block_path, 'r') as f:
        for line in f:
            line = line.strip()
            if line:
                previous_blocks.add(line)
    
    new_lines = []
    with open(block_path, 'r') as f:
        for line in f:
            line = line.strip()
            if line and line not in previous_blocks:
                new_lines.append(line)
    
    # Overwrite block_path with only the new entries (so subsequent processing can use it as-is)
    with open(block_path, 'w') as f:
        for line in new_lines:
            f.write(line + '\n')
    
    # if not new_lines:
    #     raise ValueError("No new blocks found. Skipping.")
    

# Group by average line count and write to JSON
# Store the grouped block_start_line, block_end_line, and the components contained within them
def divide(average, block_path, block_group_path, target_dir, original_dir, resume, previous_block_path):
    """
    Divide blocks into groups based on the average line count
    
    Args:
        average: Average number of lines per group
        block_path: Path to the file containing block information
        block_group_path: Path to the JSON file where grouped data will be saved
    """
    print("Dividing blocks...")

    # Read block_path line by line
    blocks = []
    with open(block_path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            
            # Parse the format "main:trans_c/mini/main.c:48:48"
            parts = line.split(':')
            if len(parts) >= 4:
                name = parts[0]

                # The file path may contain multiple colons, so the last two are line numbers
                file_path = ':'.join(parts[1:-2])
                start_line = int(parts[-2])
                end_line = int(parts[-1])
                
                blocks.append({
                    'name': name,
                    'file_path': file_path,
                    'start_line': start_line,
                    'end_line': end_line,
                    'line_count': end_line - start_line + 1
                })
    
    # Grouping (process in order regardless of file)
    result = []
    group_id = 0
    current_group = {
        'group_id': group_id,
        'block_start_line': None,
        'block_end_line': None,
        'components': [],
        'total_lines': 0
    }
    
    for block in blocks:
        # If the current group is empty, set the start line
        if current_group['block_start_line'] is None:
            current_group['block_start_line'] = block['start_line']
        
        # Add the block
        current_group['components'].append(block)
        current_group['block_end_line'] = block['end_line']
        current_group['total_lines'] += block['line_count']
        
        # If the average line count is exceeded, finalize the group
        if current_group['total_lines'] >= average:
            result.append(current_group)
            group_id += 1
            current_group = {
                'group_id': group_id,
                'block_start_line': None,
                'block_end_line': None,
                'components': [],
                'total_lines': 0
            }
    
    # If there are remaining blocks, add them as the last group
    if current_group['components']:
        result.append(current_group)
    
    # Write to JSON
    write_json(block_group_path, result)
    
    print(f"Created {len(result)} groups from {len(blocks)} blocks")
    print(f"Saved to: {block_group_path}")
    
    return result


def signal_handler(signum, frame):
    global keyboard_interrupt_occurred
    keyboard_interrupt_occurred = True
    raise KeyboardInterrupt


def show_iteration_counts(archive_dir, result_path, dep_json_path, meta_dir, database_dir, target, exec_time):
    print(f"iteration_counts")
    for rust_path in iteration_dict:
        print(f"{rust_path}: {iteration_dict[rust_path]}")
    
    end_time = time.time()
    exec_time = end_time - start_time
    print(f"Execution Time: {exec_time} seconds")

    print("----------- Translation finished -----------")

    #if not keyboard_interrupt_occurred:    
    get_compile_report(archive_dir, result_path, dep_json_path, meta_dir, database_dir, target, exec_time)



def reflect_toml_modification(rsp_json):
    # Rewrite the entire Cargo.toml
    cargo_toml_path = rust_output_dir + "/" + "Cargo.toml"
    #write_toml(rsp_json['toml'], cargo_toml_path) # Write TOML data to file

    # Instead of rewriting the entire Cargo.toml, merge into it
    existing_data = load_toml_file(cargo_toml_path)
    merge_toml_json(existing_data, rsp_json['toml']) # Overwrite existing data with new JSON data
    updated_toml_data = toml.dumps(existing_data) # Convert the overwritten data to TOML format
    write_toml_file(updated_toml_data, cargo_toml_path) # Write TOML data to file



def get_compile_report(archive_dir, result_path, dep_json_path, meta_dir, database_dir, target, exec_time):

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

    trial_id = "trial_" + str(moment_json[target]['current_count'] - 1)

    if result_json is None:
        result_json = {}
    if target not in result_json:
        result_json[target] = {}
    if trial_id not in result_json[target]:
        result_json[target][trial_id] = {}
        
    avr = moment_json[target][trial_id]['average']
    destination = archive_dir + "/" + target + "/" + trial_id + f"_{avr}"

    calculate_execution_time(chat_dir, "time.json", trial_id, target)

    copy_directory(rust_output_dir, destination)
    copy_directory(meta_dir, destination)
    copy_directory(exp_dir, destination)
    copy_directory(chat_dir, destination)

    if os.path.exists(chat_macro_dir):
        copy_directory(chat_macro_dir, destination)
    copy_file(f"{database_dir}/judge.json", destination)

    copy_file(log_file_path, destination)
    copy_file(dep_json_path, destination)
    copy_file(token_path, destination)
    copy_file(f"{database_dir}/token_macro.json", destination)
    copy_file(f"{database_dir}/time.json", destination)
    copy_file(f"{database_dir}/classify_data.json", destination)

    current_directory = os.getcwd()
    result_json[target][trial_id]['cwd'] = current_directory
    result_json[target][trial_id]['exec_time'] = exec_time
    result_json[target][trial_id]['archive_dir'] = destination
    #result_json[target][trial_id]['saved_rust_dir'] = saved_rust_dir
    #result_json[target][trial_id]['saved_meta_dir'] = saved_meta_dir
    #result_json[target][trial_id]['saved_log_dir'] = saved_log_dir
    result_json[target][trial_id]['average'] = moment_json[target][trial_id]['average']

    for category in ['conv_type', 'llm_model', 'conv_sum_prompt', 'repair_sum_prompt', 'repair_history', 'conv_show_files', 'repair_show_files', 'log_file_path']: #, 'llm_model'
        if category in moment_json[target][trial_id]:
            result_json[target][trial_id][category] = moment_json[target][trial_id][category]

    if trial_id not in result_json[target]:
        result_json[target][trial_id] = {}

    #num = result_json[target]['current_count']

    result_json[target][trial_id]['equivalence_average'] = None
    result_json[target][trial_id]['compile_average'] = None # Store the average number of compilations for all files

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

    ######### updating result.json #########
    result_json = read_json(result_path)
    moment_json = read_json(moment_path)
    trial_id = "trial_" + str(moment_json[target]['current_count'] - 1) # Same for moment_path as well

    average = moment_json[target][trial_id]['average']
    sum_repair_count = 0
    file_count = 0

    for rust_path in moment_json[target][trial_id]['paths']:
        exp_path = obtain_exp_path(rust_path, average)
        exp_data = read_json(exp_path)

        repair_count = 0  # Or default value
        sum_input = 0
        sum_output = 0

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

    if file_count == 0:
        result_json[target][trial_id]['compile_average'] = sum_repair_count
    else:
        result_json[target][trial_id]['compile_average'] = sum_repair_count / file_count

    write_json(result_path, result_json)


def read_original_files(directory):
    c_files = []
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith('.c') or file.endswith('.h'):
                c_files.append(os.path.join(root, file))
    
    return c_files


def get_c_order(block_group_path):
    c_order = []
    components = {}
    group_data = read_json(block_group_path)

    if group_data is None:
        return [], {}

    for item in group_data:
        c_order.append(str(item['group_id']))
        if str(item['group_id']) not in components:
            components[str(item['group_id'])] = item['components']

    """
    # Read block_path line by line
    blocks = []
    with open(block_path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            
            # Parse the format "main:trans_c/mini/main.c:48:48"
            parts = line.split(':')
            if len(parts) >= 4:
                name = parts[0]
                # The file path may contain multiple colons, so the last two are line numbers
                file_path = ':'.join(parts[1:-2])
                start_line = int(parts[-2])
                end_line = int(parts[-1])
                
                blocks.append({
                    'name': name,
                    'file_path': file_path,
                    'start_line': start_line,
                    'end_line': end_line,
                    'line_count': end_line - start_line + 1
                })
    """
    return c_order, components



def parse_function_info(target_path, target_dir, user_id):
    """
    Parse function information from a text file and return as JSON
    
    Args:
        target_path: Path to the text file containing function information
        
    Returns:
        List of dictionaries in JSON format
    """
    functions = []
    
    if not os.path.exists(target_path):
        raise ValueError(f"No exists: {target_path}")

    with open(target_path, 'r') as f:
        content = f.read()
    
    entries = content.split()
    
    for entry in entries:
        parts = entry.split(':')
        
        file_path = f"{parts[1]}"
        file_path = file_path.replace(f"{TRANS_HOME}/trans_c_{user_id}/", "")
        file_path = f"{target_dir}/{file_path}"

        if len(parts) >= 3: #4:
            functions.append({
                'name': parts[0],
                'file_path': file_path,
                'start_line': int(parts[2]),
                #'end_line': int(parts[3])
            })
    
    print(f"Parsed {len(functions)} item(s)")
    
    if len(functions) < 1:
        raise ValueError(f"No exists: {functions}")

    return functions


link_template = f"""# In "modify_data" mode
{{
    "mode" : "modify_data",
    "answer" : [
        {{
            "file_path" : (file path),
            "start_line" : (start line of the original code to be deleted; must reflect the original range to be replaced),
            "end_line" : (end line of the original code to be deleted; must reflect the original range to be replaced),
            "modified_data" : (content of the corrected code without any omission. Content of the corrected code as a string if is_JSON is false, or as a direct JSON object if is_JSON is true.),
        }},
        {{
            "file_path" : (file path),
            "start_line" : (start line of the original code to be deleted; must reflect the original range to be replaced),
            "end_line" : (end line of the original code to be deleted; must reflect the original range to be replaced),
            "modified_data" : (content of the corrected code without any omission. Content of the corrected code as a string if is_JSON is false, or as a direct JSON object if is_JSON is true.),
        }},...
    ],
    "ongoing" : true if ANY step remains incomplete and requires continuation. false ONLY when ALL steps (1-3) are fully completed.
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
    "ongoing" : true if ANY step remains incomplete and requires continuation. false ONLY when ALL steps (1-3) are fully completed.
    "reason" : explanatory text for the response (insert here if needed)
}}
"""

def generate_link_harness(work_dir, build_path, rust_build_path, run_test_path, run_all_path, database_dir, user_id, 
                          lib_path, rust_lib_h_path, rust_output_dir, raw_dir, target_dir, target_path, llm_interface, rust_edition):
    
    
    functions = parse_function_info(target_path, work_dir, user_id)

    prompt = [
        f"The following directory ({work_dir}) is created for calling Rust functions from a C program.",
        "Please complete the following steps:",
        #f"  Step 1. Create stub implementations of the target C functions as Rust functions in the Rust library file ({lib_path}).",
        f"  Step 1. Update the Rust library file ({lib_path}) so that it includes stub implementations of the target C functions as Rust functions.",
        #f"  Step 1. Create an empty implementation of the C FFI boundary functions as a Rust functions in the Rust library file: {lib_path}.",
        #f"  Step 2. Create the Rust library code.",
        f"  Step 2. Create a C header file ({rust_lib_h_path}) that makes the Rust library function callable.",
        f"  Step 3. Replace the target C function implementations in {target_dir} with calls to the corresponding Rust stub functions.", #f"  Step 4. Modify the C code ({target_dir}) to build it so it can be called from C."
        f"          For examples: - Include the Rust header file created in step 2.",
        f"                        - Update build scripts to link the Rust library.",
        f"                        - Comment out the target C function.",
        # "## How to modify each testcase:",
        # f"  - At the beginning of each testcase, output \"Test {{test_num}} started\" to standard output.",
        # f"  - At the end of each testcase, output \"Test {{test_num}} ended\" to standard output.",
        #f"  - Please write the answer in the following JSON format.",
    ]
    prompt.extend(["",
                   f"## Rules",
                   f"When responding, please strictly apply all the specified rules below and provide only the answer that follows the rules.",
                   "- The project layout is as follows:",
                   f"  - Shell script to build the Rust code: {rust_build_path}",
                   f"  - Shell script to build the C code: {build_path}",
                   f"  - Shell script to run the C test code: {run_test_path}",
                   f"  - Shell script to run all the scripts above: {run_all_path}",
                   f"  - Original C program directory: {target_dir}",
                   f"  - Corresponding Rust program directory: {rust_output_dir}",
                   f"- In step 1, do NOT implement the actual logic. The goal at this stage is only to make the project compile successfully.",
                   f"- In step 3, please comment out the C call section, since it's no longer necessary.",
                   f"- IMPORTANT: Do NOT overwrite existing Rust build configuration files in the Rust library ({rust_output_dir}).",
                   f"    - The {rust_output_dir} directory already contains pre-configured build files (Cargo.toml, build.rs).",
                   f"    - Do NOT modify or regenerate Cargo.toml - it already has the correct dependencies.",
                   f"    - Do NOT modify or regenerate build.rs - it already has the correct build configuration.",
                   f"    - Only create or modify the Rust source files (e.g., src/lib.rs) and C header files.",
                   f"- EXCEPTION: If the build error originates from build.rs itself (e.g., bindgen failure, missing bindings.rs generation, commented-out error messages, incorrect paths, or other build script issues), you SHOULD fix build.rs to resolve the root cause rather than working around it in lib.rs.",
                   #f"- EXCEPTION: If bindgen fails with \"redefinition\" or \"conflicting types\" errors in build.rs, you may add .blocklist_function() or .blocklist_item() calls to the existing builder chains in build.rs to resolve symbol collisions.",
                   f"      - Do NOT remove any files from config_paths. All .c files must remain in config_paths to ensure complete macro detection.",
                   f"      - Add the blocklist to BOTH builder instances in build.rs (the main bindings builder and the detect_all_macros builder).",
                   f"- If the target function is named 'main', rename it to a unique name on the Rust side by appending '_main' to the filename (stripped of its extension, with non-alphanumeric characters replaced by '_'), and prefix with the parent directory name if a collision occurs.",
                   f"- Use Rust {rust_edition} edition.",
                   #f"- Shell script to build test code: {build_path}",
    ])

    # Add each function
    prompt.extend(["",
                   f"## Target C functions for step 1:",
    ])
    for func in functions:
        prompt.append(f"   - {func['name']} (from {func['file_path']}, line {func['start_line']})")
    

    prompt.extend(["\n## Response format", "Please write the answer in the following JSON format.",  
    ])
    prompt.extend([link_template])

    prompt.extend(["", "## Directory structure of the translated Rust program:"])  
    directory_structure = get_dir_struct("translation", work_dir, None)
    prompt.extend([directory_structure, ""])

    ongoing_flag = None
    error = None
    std_out = None 
    iteration_count = 0
    progress_queue = None
    max_iterations = 10

    sum_target_list = []
    sum_slice_list = []
    sum_modified_list = []
    read_prompt = None

    while (1):
        if ongoing_flag is False:
            break

        rsp_json = ask_llm(prompt, "continue", llm_interface)

        if 'ongoing' in rsp_json:
            ongoing_flag = rsp_json['ongoing']

        if 'mode' in rsp_json:
            mode = rsp_json['mode']

            if mode == 'read_data':
                if 'answer' in rsp_json:
                    code = rsp_json['answer']
                    append_file(execute_path, code)

                if 'target_files' in rsp_json and rsp_json['target_files'] is not None:
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
                    modified_list = rsp_json['answer'] # It might be okay to put individually converted ones here
                    if not isinstance(modified_list, list):
                        modified_list = [modified_list]
                    sum_modified_list.extend(modified_list)
        
        if mode == 'modify_data':
            print(f"In mode: {mode}")
            reflect_line_modification(sum_modified_list, work_dir, database_dir)

        elif mode == 'read_data':
            print(f"In mode: {mode}")
            #output = run_read_script(execute_path, 50, True, None, "both")
            read_prompt = ["The content obtained in read_data mode is as follows.", ""]
            
            for see_path in sum_target_list:
                file_code = get_lined_code(see_path, work_dir)
                read_prompt.extend([f"- Content of the file {see_path}:"])  
                if len(file_code) == 0:
                    file_code = f"Line 1 [0]: [This {see_path} file is currently empty and contains no content. *** STOP *** Do not use read_data mode anymore.]"
                read_prompt.extend([f'{file_code}\n'])

            for see_item in sum_slice_list:
                file_code = get_lined_specific_code(database_dir, see_item['file_path'], see_item['start_line'], see_item['end_line'])
                read_prompt.extend([f"- Content of {see_item['start_line']} - {see_item['end_line']} lines in the file {see_item['file_path']}:"]) 
                read_prompt.extend([f'{file_code}\n'])

        
        prompt = [
            "Please continue your answer."
            #"The following directory is created for calling Rust functions from a C program.",
            "Please complete the following steps:",
            #f"  Step 1. Create stub implementations of the target C functions as Rust functions in the Rust library file ({lib_path}).",
            f"  Step 1. Update the Rust library file ({lib_path}) so that it includes stub implementations of the target C functions as Rust functions.",
            #f"  Step 1. Create an empty implementation of the C FFI boundary functions as a Rust functions in the Rust library file: {lib_path}.",
            #f"  Step 2. Create the Rust library code.",
            f"  Step 2. Create a C header file ({rust_lib_h_path}) that makes the Rust library function callable.",
            f"  Step 3. Replace the target C function implementations in {target_dir} with calls to the corresponding Rust stub functions.", #f"  Step 4. Modify the C code ({target_dir}) to build it so it can be called from C."
            f"          For examples: - Include the Rust header file created in step 2.",
            f"                        - Update build scripts to link the Rust library.",
            f"                        - Comment out the target C function.",
            # "## How to modify each testcase:",
            # f"  - At the beginning of each testcase, output \"Test {{test_num}} started\" to standard output.",
            # f"  - At the end of each testcase, output \"Test {{test_num}} ended\" to standard output.",
            #f"  - Please write the answer in the following JSON format.",
        ]
        prompt.extend(["",
                    f"## Rules",
                    f"When responding, please strictly apply all the specified rules below and provide only the answer that follows the rules.",
                    "- The project layout is as follows:",
                    f"  - Shell script to build the Rust code: {rust_build_path}",
                    f"  - Shell script to build the C code: {build_path}",
                    f"  - Shell script to run the C test code: {run_test_path}",
                    f"  - Shell script to run all the scripts above: {run_all_path}",
                    f"  - Original C program directory: {target_dir}",
                    f"  - Corresponding Rust program directory: {rust_output_dir}",
                    f"- In step 1, do NOT implement the actual logic. The goal at this stage is only to make the project compile successfully.",
                    f"- In step 3, please comment out the C call section, since it's no longer necessary.",
                    f"- IMPORTANT: Do NOT overwrite existing Rust build configuration files in the Rust library ({rust_output_dir}).",
                    f"    - The {rust_output_dir} directory already contains pre-configured build files (Cargo.toml, build.rs).",
                    f"    - Do NOT modify or regenerate Cargo.toml - it already has the correct dependencies.",
                    f"    - Do NOT modify or regenerate build.rs - it already has the correct build configuration.",
                    f"    - Only create or modify the Rust source files (e.g., src/lib.rs) and C header files.",
                    f"- EXCEPTION: If the build error originates from build.rs itself (e.g., bindgen failure, missing bindings.rs generation, commented-out error messages, incorrect paths, or other build script issues), you SHOULD fix build.rs to resolve the root cause rather than working around it in lib.rs.",
                    #f"- EXCEPTION: If bindgen fails with \"redefinition\" or \"conflicting types\" errors in build.rs, you may add .blocklist_function() or .blocklist_item() calls to the existing builder chains in build.rs to resolve symbol collisions.",
                    f"      - Do NOT remove any files from config_paths. All .c files must remain in config_paths to ensure complete macro detection.",
                    f"      - Add the blocklist to BOTH builder instances in build.rs (the main bindings builder and the detect_all_macros builder).",
                    f"- Use Rust {rust_edition} edition.",
                    #f"- Shell script to build test code: {build_path}",
        ])

        prompt.extend(["\n## Response format", "Please write the answer in the following JSON format.",  
        ])
        prompt.extend([link_template])

        if read_prompt is not None:
            prompt.extend(["", "## Response to the previous request:"])
            prompt.extend(read_prompt)
            read_prompt = None # Initialization

    print("*********** End of reforamt ***********")


code_snippet = f"""```rust
// Rust side
use std::ffi::CStr;
use std::os::raw::c_char;

unsafe fn parse_args(argc: i32, argv: *const *const c_char) -> Vec<String> {{
    (0..argc as usize)
        .map(|i| {{
            let ptr = *argv.add(i);
            if ptr.is_null() {{
                String::new()
            }} else {{
                CStr::from_ptr(ptr).to_string_lossy().into_owned()
            }}
        }})
        .collect()
}}

fn rust_main_<identifer>(args: Vec<String>) -> i32 {{
    // Stub implementation

    use std::io::Write;
    std::io::stdout().flush().unwrap();
    0
}}

#[unsafe(no_mangle)]
pub extern "C" fn rust_main_wrapper_<identifer>(
    argc: i32,
    argv: *const *const c_char,
) -> i32 {{
    let args = unsafe {{ parse_args(argc, argv) }};
    rust_main_<identifer>(args)
}}
```"""

# When translating the C main function, obtain command-line arguments using std::env::args() in Rust instead of receiving argc/argv from C.
def generate_link_harness_minimize(work_dir, build_path, rust_build_path, run_test_path, run_all_path, database_dir, user_id,
                          lib_path, rust_lib_h_path, rust_output_dir, raw_dir, target_dir, target_path, llm_interface, rust_edition):
    
    functions = parse_function_info(target_path, work_dir, user_id)

    prompt = [
        f"The following directory ({work_dir}) is created for calling Rust functions from a C program.",
        "Please complete the following steps:",
        f"  Step 1. Update the Rust library file ({lib_path}) so that it includes stub implementations of the Rust main wrapper function, i.e., create pub extern \"C\" fn rust_main_wrapper_<identifier>(argc: i32, argv: *const *const std::os::raw::c_char) -> i32. Also create a safe parse_args function and a rust_main_<identifier>(args: Vec<String>) -> i32 function.",
        f"          Note that <identifier> is derived from the C source file containing the corresponding main() function and must be unique across all main() functions listed under 'Target C main functions for step 1', so that each main() maps to a distinct rust_main_<identifier> / rust_main_wrapper_<identifier> pair with no name collisions; however, if there is only one main() function in the project, omit the <identifier> suffix entirely and use rust_main / rust_main_wrapper.",
        f"  Step 2. Create a C header file ({rust_lib_h_path}) that makes the Rust library function callable by writing extern int rust_main_wrapper_<identifier>(int argc, char *argv[]);.",
        f"  Step 3. Replace the C main() function implementations in {target_dir} with a thin wrapper: int main(int argc, char *argv[]) {{ return rust_main_wrapper_<identifier>(argc, argv); }}",
        f"  Step 4. Update build scripts to link the Rust library. Include the Rust header file created in step 2. Also, add the Rust static library file as a build dependency of C binary targets, so that C binaries are automatically relinked when the Rust library is updated."    
    ]

    prompt.extend(["",
                   f"## Rules",
                   f"When responding, please strictly apply all the specified rules below and provide only the answer that follows the rules.",
                   "- The project layout is as follows:",
                   f"  - Shell script to build the Rust code: {rust_build_path}",
                   f"  - Shell script to build the C code: {build_path}",
                   f"  - Shell script to run the C test code: {run_test_path}",
                   f"  - Shell script to run all the scripts above: {run_all_path}",
                   f"  - Original C program directory: {target_dir}",
                   f"  - Corresponding Rust program directory: {rust_output_dir}",
                   #f"- In step 1, do NOT implement the actual logic. The goal at this stage is only to make the project compile successfully.",
                   f"- In step 1, the stub of rust_main_<identifier> should be an empty function body. Do NOT call back into any C function. Do NOT declare extern blocks to call C functions.",
                   f"- In step 1, the Rust library must define three functions:",
                   f"    - unsafe fn parse_args(argc: i32, argv: *const *const std::os::raw::c_char) -> Vec<String> — converts C argc/argv to safe Rust Vec<String>.",
                   f"    - fn rust_main_<identifier>(args: Vec<String>) -> i32 — the main logic, entirely safe Rust. The stub should have an empty body that calls std::io::stdout().flush() and returns 0.", #The stub should have an empty body returning 0.",
                   #f"    - rust_main_<identifier> must call `use std::io::Write; std::io::stdout().flush().unwrap();` before returning, to flush Rust's stdout buffer before control returns to C.",
                   f"    - #[unsafe(no_mangle)] pub extern \"C\" fn rust_main_wrapper_<identifier>(argc: i32, argv: *const *const std::os::raw::c_char) -> i32 — the FFI entry point that calls parse_args then rust_main_<identifer>.",
                   f"    - In short, the lib.rs output for step 1 MUST match the following template exactly:", f"{code_snippet}",
                   f"- The unsafe code must be confined to parse_args only. rust_main_<identifier> must be 100% safe Rust.",
                   #f"- In step 3, please comment out the C call section, since it's no longer necessary.",
                   f"- IMPORTANT: Do NOT overwrite existing Rust build configuration files in the Rust library ({rust_output_dir}).",
                   f"    - The {rust_output_dir} directory already contains pre-configured build files (Cargo.toml, build.rs).",
                   f"    - Do NOT modify or regenerate Cargo.toml - it already has the correct dependencies.",
                   f"    - Do NOT modify or regenerate build.rs - it already has the correct build configuration.",
                   f"    - Only create or modify the Rust source files (e.g., src/lib.rs) and C header files.",
                   f"- EXCEPTION: If the build error originates from build.rs itself (e.g., bindgen failure, missing bindings.rs generation, commented-out error messages, incorrect paths, or other build script issues), you SHOULD fix build.rs to resolve the root cause rather than working around it in lib.rs.",
                   #f"- EXCEPTION: If bindgen fails with \"redefinition\" or \"conflicting types\" errors in build.rs, you may add .blocklist_function() or .blocklist_item() calls to the existing builder chains in build.rs to resolve symbol collisions.",
                   f"      - Do NOT remove any files from config_paths. All .c files must remain in config_paths to ensure complete macro detection.",
                   f"      - Add the blocklist to BOTH builder instances in build.rs (the main bindings builder and the detect_all_macros builder).",
                   #f"- If the target function is named 'main', rename it to a unique name on the Rust side by appending '_main' to the filename (stripped of its extension, with non-alphanumeric characters replaced by '_'), and prefix with the parent directory name if a collision occurs.",
                   f"- Use Rust {rust_edition} edition.",
                   #f"- Shell script to build test code: {build_path}",
    ])

    # Add each function
    prompt.extend(["",
                   f"## Target C main functions for step 1:",
    ])
    for func in functions:
        prompt.append(f"   - {func['name']} (from {func['file_path']}, line {func['start_line']})")
    

    prompt.extend(["\n## Response format", "Please write the answer in the following JSON format.",  
    ])
    prompt.extend([link_template])

    prompt.extend(["", "## Directory structure of the translated Rust program:"])  
    directory_structure = get_dir_struct("translation", work_dir, None)
    prompt.extend([directory_structure, ""])

    ongoing_flag = None
    error = None
    std_out = None 
    iteration_count = 0
    progress_queue = None
    max_iterations = 10

    sum_target_list = []
    sum_slice_list = []
    sum_modified_list = []
    read_prompt = None

    while (1):
        if ongoing_flag is False:
            break

        rsp_json = ask_llm(prompt, "continue", llm_interface)

        if 'ongoing' in rsp_json:
            ongoing_flag = rsp_json['ongoing']

        if 'mode' in rsp_json:
            mode = rsp_json['mode']

            if mode == 'read_data':
                if 'answer' in rsp_json:
                    code = rsp_json['answer']
                    append_file(execute_path, code)

                if 'target_files' in rsp_json and rsp_json['target_files'] is not None:
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
                    modified_list = rsp_json['answer'] # It might be okay to put individually converted ones here
                    if not isinstance(modified_list, list):
                        modified_list = [modified_list]
                    sum_modified_list.extend(modified_list)
        
        if mode == 'modify_data':
            print(f"In mode: {mode}")
            reflect_line_modification(sum_modified_list, work_dir, database_dir)

        elif mode == 'read_data':
            print(f"In mode: {mode}")
            read_prompt = ["The content obtained in read_data mode is as follows.", ""]
            
            for see_path in sum_target_list:
                file_code = get_lined_code(see_path, work_dir)
                read_prompt.extend([f"- Content of the file {see_path}:"])  
                if len(file_code) == 0:
                    file_code = f"Line 1 [0]: [This {see_path} file is currently empty and contains no content. *** STOP *** Do not use read_data mode anymore.]"
                read_prompt.extend([f'{file_code}\n'])

            for see_item in sum_slice_list:
                file_code = get_lined_specific_code(database_dir, see_item['file_path'], see_item['start_line'], see_item['end_line'])
                read_prompt.extend([f"- Content of {see_item['start_line']} - {see_item['end_line']} lines in the file {see_item['file_path']}:"]) 
                read_prompt.extend([f'{file_code}\n'])

        prompt = [
            #"The following directory is created for calling Rust functions from a C program.",
            "Please complete the following steps:",
            #f"  Step 1. Create stub implementations of the target C functions as Rust functions in the Rust library file ({lib_path}).",
            #f"  Step 1. Create stub implementations of the Rust main function in the Rust library file ({lib_path}), i.e., create #[no_mangle] pub extern \"C\" fn rust_main_<identifer>() with no arguments. Command-line arguments will be obtained via std::env::args() in Rust, not passed from C.",
            #f"  Step 1. Update the Rust library file ({lib_path}) so that it includes stub implementations of the Rust main function, i.e., create #[no_mangle] pub extern \"C\" fn rust_main_<identifer>() with no arguments. Command-line arguments will be obtained via std::env::args() in Rust, not passed from C.",
            f"  Step 1. Update the Rust library file ({lib_path}) so that it includes stub implementations of #[unsafe(no_mangle)] pub extern \"C\" fn rust_main_wrapper_<identifier>(argc: i32, argv: *const *const std::os::raw::c_char) -> i32, a safe parse_args function, and rust_main_<identifier>(args: Vec<String>) -> i32. These signatures are FROZEN — only the body of rust_main_<identifier> may be modified.",
            #f"  Step 1. Create an empty implementation of the C FFI boundary functions as a Rust functions in the Rust library file: {lib_path}.",
            #f"  Step 2. Create the Rust library code.",
            f"  Step 2. Create a C header file ({rust_lib_h_path}) that makes the Rust library function callable.",
            f"  Step 3. Replace the target C function implementations in {target_dir} with calls to the corresponding Rust stub functions.", 
            f"          For examples: - Include the Rust header file created in step 2.",
            f"                        - Update build scripts to link the Rust library.",
            f"                        - Comment out the target C function.",
            # "## How to modify each testcase:",
            # f"  - At the beginning of each testcase, output \"Test {{test_num}} started\" to standard output.",
            # f"  - At the end of each testcase, output \"Test {{test_num}} ended\" to standard output.",
            #f"  - Please write the answer in the following JSON format.",
        ]

        prompt.extend(["",
                    f"## Rules",
                    f"When responding, please strictly apply all the specified rules below and provide only the answer that follows the rules.",
                    "- The project layout is as follows:",
                    f"  - Shell script to build the Rust code: {rust_build_path}",
                    f"  - Shell script to build the C code: {build_path}",
                    f"  - Shell script to run the C test code: {run_test_path}",
                    f"  - Shell script to run all the scripts above: {run_all_path}",
                    f"  - Original C program directory: {target_dir}",
                    f"  - Corresponding Rust program directory: {rust_output_dir}",
                    f"- In step 1, do NOT implement the actual logic. The goal at this stage is only to make the project compile successfully.",
                    f"- In step 3, please comment out the C call section, since it's no longer necessary.",
                    f"- IMPORTANT: Do NOT overwrite existing Rust build configuration files in the Rust library ({rust_output_dir}).",
                    f"    - The {rust_output_dir} directory already contains pre-configured build files (Cargo.toml, build.rs).",
                    f"    - Do NOT modify or regenerate Cargo.toml - it already has the correct dependencies.",
                    f"    - Do NOT modify or regenerate build.rs - it already has the correct build configuration.",
                    f"    - Only create or modify the Rust source files (e.g., src/lib.rs) and C header files.",
                    f"- EXCEPTION: If the build error originates from build.rs itself (e.g., bindgen failure, missing bindings.rs generation, commented-out error messages, incorrect paths, or other build script issues), you SHOULD fix build.rs to resolve the root cause rather than working around it in lib.rs.",
                    #f"- EXCEPTION: If bindgen fails with \"redefinition\" or \"conflicting types\" errors in build.rs, you may add .blocklist_function() or .blocklist_item() calls to the existing builder chains in build.rs to resolve symbol collisions.",
                    f"      - Do NOT remove any files from config_paths. All .c files must remain in config_paths to ensure complete macro detection.",
                    f"      - Add the blocklist to BOTH builder instances in build.rs (the main bindings builder and the detect_all_macros builder).",
                    f"- Use Rust {rust_edition} edition.",
                    #f"- Shell script to build test code: {build_path}",
        ])

        prompt.extend(["\nPlease write the answer in the following JSON format.",  
        ])
        prompt.extend([link_template])

        if read_prompt is not None:
            prompt.extend(["", "## Response to the previous request:"])
            prompt.extend(read_prompt)
            read_prompt = None # Initialization

    print("*********** End of reforamt ***********")



def extract_functions_from_header_with_clang(header_path):
    """Extract function declarations from rust_lib.h using clang"""
    
    if not os.path.exists(header_path):
        return []
    
    # Get AST with clang -Xclang -ast-dump=json
    result = subprocess.run(
        ["clang", "-Xclang", "-ast-dump=json", "-fsyntax-only", header_path],
        capture_output=True,
        text=True
    )
    
    if result.returncode != 0:
        print(f"Warning: clang failed to parse {header_path}")
        return []
    
    try:
        ast = json.loads(result.stdout)
    except json.JSONDecodeError:
        print(f"Warning: Failed to parse clang AST output")
        return []
    
    functions = []
    
    def find_functions(node):
        if isinstance(node, dict):
            # FunctionDecl originating from this file
            if node.get("kind") == "FunctionDecl":
                name = node.get("name")
                loc = node.get("loc", {})
                file = loc.get("file", "")
                
                # Only extract functions originating from this header file
                if name and (not file or file == header_path or header_path.endswith(file)):
                    functions.append(name)
            
            # Recursively traverse child nodes
            for key, value in node.items():
                if key == "inner" and isinstance(value, list):
                    for child in value:
                        find_functions(child)
                elif isinstance(value, dict):
                    find_functions(value)
    
    find_functions(ast)
    
    # Remove duplicates
    functions = list(set(functions))
    
    print(f"Extracted {len(functions)} function(s) from {header_path}: {functions}")
    
    return functions


def update_blocklist(rust_lib_h_path, build_rs_path):
    ######################################
    ## block function inclusion
    ######################################

    # Extract function names from rust_lib.h
    blocklist_functions = extract_functions_from_header_with_clang(rust_lib_h_path)
    
    # Generate blocklist code
    blocklist_code = generate_blocklist_code(blocklist_functions)
    
    # Load the template
    with open(build_rs_path, 'r') as f:
        content = f.read()
    
    # Replace the placeholder
    content = content.replace("// BLOCKLIST_PLACEHOLDER\n        ", blocklist_code)
    
    # Write back
    with open(build_rs_path, 'w') as f:
        f.write(content)

        

def update_global_metadata(target, div_meta_dir, database_dir, global_path, is_program_path, user_id):

    global_vars = read_json(global_path)

    appearances = {}
    apps_by_files = {}
    program_files = set(read_json(is_program_path))
    updates = []

    for item in global_vars:
        name = item['var_name']

        file_path = item['definition']['file_path']
        start_line = item['definition']['start_line']
        start_column = item['definition']['start_column']

        file_path = f"{TRANS_HOME}/trans_c_{user_id}/{file_path}"

        c_file_path = file_path.replace(f"trans_c_{user_id}", f"workspace_{user_id}_{target}")
        if is_system_file(c_file_path, program_files):
            continue
        
        if file_path not in apps_by_files:
            apps_by_files[file_path] = []

        apps_by_files[file_path].append({
            "name" : name,
            #"macro_type" : macro_type,
            "file_path" : file_path, 
            "start_line" : start_line,
            "start_column" : start_column
        })

    for file_path, defs in apps_by_files.items():
        meta_data, meta_path = obtain_metadata(file_path, div_meta_dir, False, None, "def")

        for item in defs:
            name = item['name']
            """
            print(name)
            if macro_type == "flag":
                name = "IFDEF"
            """
            def_file_path = item['file_path']
            def_start_line = item['start_line']
            def_start_column = item['start_column']
            def_key = f"{name}:{def_file_path}:{def_start_line}"

            if def_key not in meta_data:
                continue

            if 'rust_code' not in meta_data[def_key]:
                meta_data[def_key]['rust_code'] = {
                    "file_path" : None,
                    "start_line" : None,
                    "content" : None
                }
            meta_data[def_key]['rust_code']['content'] = "// Generated by bindgen in lib.rs"
            updates.append(item)
        write_json(meta_path, meta_data)

    write_json(f"{database_dir}/updates_global.json", updates)



def update_build_rs_metadata(target, user_id, div_meta_dir, database_dir, taken_macros_path, independent_const_build_path, flag_build_path, is_program_path):

    taken_macros = read_json(taken_macros_path)
    ind_const_macros = read_json(independent_const_build_path)
    flag_macros = read_json(flag_build_path)

    target_macros = []
    target_macros.extend(ind_const_macros['independent'])
    target_macros.extend(flag_macros['flags'])
    target_macros = list(set(target_macros))

    appearances = {}
    apps_by_files = {}
    program_files = set(read_json(is_program_path))
    updates = []

    for macro_key, macro in taken_macros.items():
        name = macro['name']
        if name not in target_macros:
            continue

        file_path = macro['definition']['file_path']
        start_line = macro['definition']['start_line']
        start_column = macro['definition']['start_column']

        file_path = f"{TRANS_HOME}/trans_c_{user_id}/{file_path}"

        c_file_path = file_path.replace(f"trans_c_{user_id}", f"workspace_{user_id}_{target}")
        if is_system_file(c_file_path, program_files):
            continue
        
        if file_path not in apps_by_files:
            apps_by_files[file_path] = []

        if name in ind_const_macros['independent']:
            macro_type = "independent_const"
        else:
            macro_type = "flag"

        apps_by_files[file_path].append({
            "name" : name,
            "macro_type" : macro_type,
            "file_path" : file_path, 
            "start_line" : start_line,
            "start_column" : start_column
        })

    for file_path, defs in apps_by_files.items():
        meta_data, meta_path = obtain_metadata(file_path, div_meta_dir, False, None, "def")

        for item in defs:
            name = item['name']
            """
            print(name)
            if macro_type == "flag":
                name = "IFDEF"
            """
            def_file_path = item['file_path']
            def_start_line = item['start_line']
            def_start_column = item['start_column']
            macro_type = item['macro_type']
            macro_key = f"{name}:{def_file_path}:{def_start_line}"

            if macro_key not in meta_data:
                #raise ValueError("Something is wrong here, must check later")
                continue

            if 'rust_code' not in meta_data[macro_key]:
                meta_data[macro_key]['rust_code'] = {
                    "file_path" : None,
                    "start_line" : None,
                    "content" : None
                }
            meta_data[macro_key]['rust_code']['content'] = "// Generated by bindgen in build.rs"
            updates.append(item)
        write_json(meta_path, meta_data)

    write_json(f"{database_dir}/updates.json", updates)



def generate_build_rs(build_template_path, build_rs_path, rust_lib_h_path, dep_json_path, 
                      flag_path, target_dir, is_program_path, 
                      independent_const_build_path, flag_build_path, clang_args_json_path):

    copy_file(build_template_path, build_rs_path)

    ######################################
    ## header inclusion
    ######################################
    entries = get_entry_points(target_dir, is_program_path)

    print(target_dir)
    print(is_program_path)

    if not entries:
        print("Warning: No entries provided. Generating build.rs with empty header list.")
        entries = []
        raise ValueError("Warning: No entries provided. Generating build.rs with empty header list.")
    
    # Read template file
    try:
        with open(build_rs_path, 'r') as f:
            template_content = f.read()
    except FileNotFoundError:
        print(f"Error: Template file not found: {build_rs_path}")
        return
    
    # Generate the new header paths code
    new_header_code = generate_header_paths_rust_code(entries)
    
    # Replace the config_paths section in the template
    # This regex pattern matches the entire vec! declaration including comments
    pattern = r'let config_paths = vec!\[\s*\];'
    pattern = r'let config_paths: Vec<&str> = vec!\[\s*\];'
    if not re.search(pattern, template_content):
        # If the simple pattern doesn't match, try a more flexible one
        pattern = r'let config_paths = vec!\[[\s\S]*?\];'
        pattern = r'let config_paths: Vec<&str> = vec!\[[\s\S]*?\];'

    modified_content = re.sub(pattern, new_header_code, template_content, count=1)

    # Write the modified content to build.rs
    with open(build_rs_path, 'w') as f:
        f.write(modified_content)

    ######################################
    ## clang flags inclusion
    ######################################

    compile_commands_dir = find_compile_commands_json(target_dir)
    compile_commands_path = os.path.join(compile_commands_dir, "compile_commands.json")

    with open(compile_commands_path, 'r') as f:
        data = json.load(f)

    file_clang_args = {}
    for entry in data:
        directory = entry.get("directory", "")
        file_path = os.path.normpath(os.path.join(directory, entry["file"]))
        args = []
        for arg in entry.get("arguments", []):
            if arg.startswith("-D"):
                args.append(arg)
            elif arg.startswith("-I"):
                include_path = arg[2:]
                abs_path = os.path.normpath(os.path.join(directory, include_path))
                args.append(f"-I{abs_path}")
        file_clang_args[file_path] = args

    write_json(clang_args_json_path, file_clang_args)
    
    # Read content here
    with open(build_rs_path, 'r') as f:
        content = f.read()

    content = content.replace(
        "// CLANG_ARGS_JSON_PATH_PLACEHOLDER",
        os.path.abspath(clang_args_json_path)
    )
    """
    clang_args = set()
    for entry in data:
        directory = entry.get("directory", "")
        for arg in entry.get("arguments", []):
            if arg.startswith("-D"):
                clang_args.add(arg)
            elif arg.startswith("-I"):
                include_path = arg[2:]
                abs_path = os.path.normpath(os.path.join(directory, include_path))
                clang_args.add(f"-I{abs_path}")
    
    clang_args = sorted(clang_args)

    with open(build_rs_path, 'r') as f:
        content = f.read()

    # clang_arg_code = "\n".join(f'        .clang_arg("{arg}")' for arg in clang_args)
    clang_arg_code = "\n".join(f'        builder = builder.clang_arg("{arg}");' for arg in clang_args)

    content = content.replace(
        "// CLANG_ARGS_PLACEHOLDER",
        clang_arg_code
    )
    """

    ######################################
    ## const macros  
    ######################################
    # independent_const_build_path
    content = content.replace(
        "// CONST_MACRO_PATH_PLACEHOLDER",
        os.path.abspath(independent_const_build_path)
    )

    ######################################
    ## flag macros
    ######################################
    # flag_build_path
    content = content.replace(
        "// FLAG_MACRO_PATH_PLACEHOLDER",
        os.path.abspath(flag_build_path)
    )

    ######################################
    ## Adjust file paths
    ######################################
    content = re.sub(
        rf'{TRANS_HOME}/workspace_\d+_[^/]+/',
        '../',
        content
    )

    content = re.sub(
        rf'{TRANS_HOME}/database_(\d+)/',
        r'../../database_\1/',
        content
    )
    with open(build_rs_path, 'w') as f:
        f.write(content)


    with open(clang_args_json_path) as f:
        data = json.load(f)

    new = {}
    for key, args in data.items():
        new_key = re.sub(rf'{TRANS_HOME}/workspace_\d+_[^/]+/', '../', key)
        seen, new_args = set(), []
        for a in args:
            a = re.sub(rf'{TRANS_HOME}/workspace_\d+_[^/]+/', '../', a)
            if a not in seen:                          
                new_args.append(a); seen.add(a)
        new[new_key] = new_args

    with open(clang_args_json_path, "w") as f:
        json.dump(new, f, indent=4)



# Build the Rust project and extract cargo:warning and cargo:rustc-cfg
def get_build_rs_config(rust_output_dir, build_config_path):
    
    print(rust_output_dir)

    # Get detailed output with cargo build -vv
    result = subprocess.run(
        ["cargo", "build", "-vv"],
        cwd=rust_output_dir, #run_all_path,
        capture_output=True,
        text=True
    )
    
    output = result.stdout + result.stderr
    
    # Extract detected macros (from warning lines)
    macro_pattern = re.compile(r"warning:.*Detected macro: (\w+)")
    macros = sorted(set(macro_pattern.findall(output)))
    
    # Extract cfg flags that were actually set
    cfg_pattern = re.compile(r"cargo:rustc-cfg=(\S+)")
    cfg_flags = sorted(set(cfg_pattern.findall(output)))
    
    # Create the output directory
    os.makedirs(rust_output_dir, exist_ok=True)
    
    # Write both to a single file
    with open(build_config_path, "w") as f:
        f.write("# Detected Macros\n\n")
        for macro in macros:
            f.write(f"{macro}\n")
        
        f.write("\n# Rust cfg Flags (actually set via cargo:rustc-cfg)\n\n")
        for flag in cfg_flags:
            f.write(f"{flag}\n")
    
    print(f"Detected {len(macros)} macros")
    print(f"Set {len(cfg_flags)} cfg flags")
    print(f"Written to: {build_config_path}")


def generate_global_code(global_path, lib_path):

    code_text = read_file(lib_path)
    global_vars = read_json(global_path)

    extern_block = ""
    functions_block = ""

    if global_vars is None:
        return

    for item in global_vars:
        name = item["var_name"]
        rust_type = item["rust_type"]

        extern_block += f"    static mut {name}: {rust_type};\n"

        # Getter name: get_xxx, setter name: set_xxx
        getter = f"get_{name}"
        setter = f"set_{name}"

        functions_block += f"""
pub fn {getter}() -> {rust_type} {{
    unsafe {{ {name} }}
}}

pub fn {setter}(val: {rust_type}) {{
    unsafe {{ {name} = val; }}
}}
"""

    code_text += f'\nextern "C" {{\n{extern_block}}}\n{functions_block}'

    with open(lib_path, "w") as f:
        f.write(code_text)



def turn_off_warning(build_rs_path):
    print("Turn off warnings...")
    
    # Read the file
    with open(build_rs_path, 'r') as f:
        content = f.read()
    
    # Comment out or remove lines containing cargo:warning=
    lines = content.split('\n')
    new_lines = []
    
    for line in lines:
        if 'cargo:warning=' in line:
            # If commenting out
            new_lines.append('// ' + line.lstrip())
            # Or do nothing if completely removing
            # continue
        else:
            new_lines.append(line)
    
    # Write back to the file
    with open(build_rs_path, 'w') as f:
        f.write('\n'.join(new_lines))
    
    print(f"Warnings turned off in: {build_rs_path}")


def generate_blocklist_code(functions):
    """Generate code for calling blocklist_function"""
    if not functions:
        return ""
    
    lines = [f'.blocklist_function("{func}")' for func in functions]
    return "\n        ".join(lines) + "\n        "


def check_cargo_modules_installed():
    """
    Check whether cargo-modules is installed
    
    Returns:
        bool: True if installed, otherwise False
    """
    # Method 1: Run cargo modules --version
    try:
        result = subprocess.run(
            ['cargo', 'modules', '--version'],
            capture_output=True,
            text=True,
            timeout=10
        )
        if result.returncode == 0:
            print(f"cargo-modules is installed: {result.stdout.strip()}")
            return True
    except (subprocess.SubprocessError, FileNotFoundError):
        pass
    
    # Method 2: Check with cargo --list
    try:
        result = subprocess.run(
            ['cargo', '--list'],
            capture_output=True,
            text=True,
            timeout=10
        )
        if result.returncode == 0 and 'modules' in result.stdout:
            print("cargo-modules is installed (found in cargo --list)")
            return True
    except (subprocess.SubprocessError, FileNotFoundError):
        pass
    
    print("cargo-modules is NOT installed")
    return False


def insert_is_target(target_dir, marker, meta_dir, div_meta_dir):
    """
    Recursively search files under target_dir,
    detect lines containing marker comments
    (/* Here is one target function: ... */),
    and display the file path, line number, and function name.

    Returns:
        list[dict]: List of detected marker information
            Each element: {"file_path": str, "line_num": int, "func_name": str, "raw_line": str}
    """
    # Marker pattern: /* Here is one target function: <file_path>:<line_num>:<name> */
    pattern = re.compile(
        r'/\*\s*Here is one target function:\s*(.+?):(\d+):(\S+?)\s*\*/'
    )

    results = []

    target_dir = os.path.abspath(target_dir)

    for root, dirs, files in os.walk(target_dir):
        for filename in files:
            file_path = os.path.join(root, filename)

            # Skip binary files
            try:
                with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                    lines = f.readlines()
            except (IOError, OSError):
                continue

            for line_idx, line in enumerate(lines, start=1):
                match = pattern.search(line)
                if match:
                    embedded_path = match.group(1)
                    embedded_line = int(match.group(2))
                    func_name = match.group(3)

                    results.append({
                        "file_path": file_path,          # File where the marker was actually found
                        "line_num": line_idx,             # Actual line number
                        "embedded_path": embedded_path,   # Path embedded in the marker
                        "embedded_line": embedded_line,    # Line number embedded in the marker
                        "func_name": func_name,
                    })

    markers_by_file = defaultdict(list)
    for r in results:
        markers_by_file[r['file_path']].append(r)

    for file_path, markers in markers_by_file.items():
        div_meta_data, div_meta_path = obtain_metadata(file_path, div_meta_dir, False, None, "def")
        if div_meta_data is None:
            raise ValueError(f"Cannot find {div_meta_path} for {file_path}. You might need to check the path of {target_path}. Please check whether the target location is included in the compile_commands.json.")
        for key_name, item in div_meta_data.items():
            for m in markers:
                if item['block_start'] <= m['line_num'] <= item['block_end'] and item['name'] == m['func_name']:
                    item['is_target'] = True
        write_json(div_meta_path, div_meta_data)

    
    """
    # Display the results
    print(f"=== Marker detection results ({len(results)} items) ===")
    for i, r in enumerate(results, start=1):
        file_path = r['file_path']
        line_num = r['line_num']
        name = r['func_name']
        # print(f"    (embedded path: {r['embedded_path']}, embedded line: {r['embedded_line']})")
        # print()

        # meta_data, meta_path = obtain_metadata(file_path, meta_dir, False, None, "def")
        div_meta_data, div_meta_path = obtain_metadata(file_path, div_meta_dir, False, None, "def")

        for key_name, item in div_meta_data.items():
            if item['block_start'] <= line_num and line_num <= item['block_end']:
                item['is_target'] = True
                
        #write_json(meta_path, meta_data)
        write_json(div_meta_path, div_meta_data)
    """

    return results


def setup_build(translation_type, list_path, dep_json_path, meta_dir, div_meta_dir, raw_dir, work_dir, target_dir, database_dir, 
                        chat_dir, original_dir, c_code_dir, rust_output_dir, logging_path, count_path, token_path, history_path, moment_path, log_dir,
                        average, log_file_path, cfg_path, flag_path, build_config_path, rust_edition,
                        run_test_path, run_all_path, build_path, rust_lib_h_path, rust_build_path, target, user_id,
                        time_path, map_path, block_path, block_group_path, progress_queue, max_iterations, llm_interface,
                        rust_c_path, c_rust_path, build_template_path, run_all_template_path, target_path, global_path,
                        is_program_path, resume, previous_block_path, marker, trial_id, taken_macros_path,
                        independent_const_build_path, flag_build_path, clang_args_json_path):

    divide(average, block_path, block_group_path, target_dir, original_dir, resume, previous_block_path) 
    print(f"c_run_path is {run_test_path}, rust_build_path is {rust_build_path}")

    # create rust library dir and rust_build_path
    build_rs_path, lib_path, toml_path = create_rust_libdir(work_dir, rust_output_dir) 
    create_rust_build_path(rust_build_path)

    insert_is_target(original_dir, marker, meta_dir, div_meta_dir)

    #####################################
    ## Pre-compilation check
    #####################################

    print("\n========= Pre-compilation check =========\n")

    # insert build.rs
    generate_build_rs(build_template_path, build_rs_path, rust_lib_h_path, dep_json_path, 
                      flag_path, target_dir, is_program_path, 
                      independent_const_build_path, flag_build_path, clang_args_json_path) # copy_file(build_template_path, build_rs_path)  #

    # update metadata for build.rs
    update_build_rs_metadata(target, user_id, div_meta_dir, database_dir, taken_macros_path, independent_const_build_path, flag_build_path, is_program_path)

    # turn off warnings
    # turn_off_warning(build_rs_path)

    # insert Cargo.toml
    generate_cargo_toml(toml_path)

    # insert run_all.sh
    generate_run_all_path(run_all_path, run_all_template_path, target)

    # insert Rust library link code
    generate_link_harness(work_dir, build_path, rust_build_path, run_test_path, run_all_path, database_dir, user_id, 
                          lib_path, rust_lib_h_path, rust_output_dir, raw_dir, target_dir, target_path, llm_interface, rust_edition)

    # update blocklist
    update_blocklist(rust_lib_h_path, build_rs_path)

    """
    # insert global variables 
    generate_global_code(global_path, lib_path)

    update_global_metadata(target, div_meta_dir, database_dir, global_path, is_program_path, user_id)
    """
    rust_path = lib_path
    
    repair_count = 1
    interface = {
        "trial_id" : trial_id,
        "build_path" : build_path,
        "lib_path" : lib_path,
        "exp_data" : None,
        "rust_build_path" : rust_build_path,
        "target_dir" : raw_dir,
        "list_path" : list_path,
        "meta_dir" : meta_dir,
        "dep_json_path" : dep_json_path,
        "repair_count" : repair_count
    }

    exp_data = {}
    execute_path = f"{work_dir}/execute.sh"
    explore_time = 0
    notes = []

    interface = TransConfig(
        rust_path=rust_path,
        rust_c_path=rust_c_path,
        c_rust_path=c_rust_path,
        target_path=target_path,
        raw_dir=raw_dir,
        rust_output_dir=rust_output_dir,
        select=False,
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
        rust_build_path=rust_build_path,
        run_test_path=run_test_path,
        run_all_path=run_all_path,
        meta_dir=meta_dir,
        div_meta_dir=div_meta_dir,
        dep_json_path=dep_json_path,
        exp_data=exp_data,
        repair_count=repair_count,
        rust_edition=rust_edition,
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
        max_iterations=max_iterations,
    )

    modified_c_keys = repair_execute('compile', interface)

    # listup configuration
    get_build_rs_config(rust_output_dir, build_config_path)
    
    return build_rs_path, lib_path, toml_path


def setup_build_minimize(translation_type, list_path, dep_json_path, meta_dir, div_meta_dir, raw_dir, work_dir, target_dir, database_dir, 
                        chat_dir, original_dir, c_code_dir, rust_output_dir, logging_path, count_path, token_path, history_path, moment_path, log_dir,
                        average, log_file_path, cfg_path, flag_path, build_config_path, rust_edition,
                        run_test_path, run_all_path, build_path, rust_lib_h_path, rust_build_path, target, user_id,
                        time_path, map_path, block_path, block_group_path, progress_queue, max_iterations, llm_interface,
                        rust_c_path, c_rust_path, build_template_path, run_all_template_path, target_path, global_path,
                        is_program_path, resume, previous_block_path, marker, trial_id, taken_macros_path,
                        independent_const_build_path, flag_build_path, clang_args_json_path):

    divide(average, block_path, block_group_path, target_dir, original_dir, resume, previous_block_path) 
    print(f"c_run_path is {run_test_path}, rust_build_path is {rust_build_path}")

    # create rust bin dir and rust_build_path
    build_rs_path, lib_path, toml_path = create_rust_libdir(work_dir, rust_output_dir) 
    create_rust_build_path(rust_build_path)

    insert_is_target(original_dir, marker, meta_dir, div_meta_dir)

    #####################################
    ## Pre-compilation check
    #####################################

    print("\n========= Pre-compilation check =========\n")

    # insert build.rs
    generate_build_rs(build_template_path, build_rs_path, rust_lib_h_path, dep_json_path, 
                      flag_path, target_dir, is_program_path, 
                      independent_const_build_path, flag_build_path, clang_args_json_path)

    # update metadata for build.rs
    update_build_rs_metadata(target, user_id, div_meta_dir, database_dir, taken_macros_path, independent_const_build_path, flag_build_path, is_program_path)

    # turn off warnings
    # turn_off_warning(build_rs_path)

    # insert Cargo.toml
    generate_cargo_toml(toml_path)

    # insert run_all.sh
    generate_run_all_path(run_all_path, run_all_template_path, target)

    # insert Rust library link code
    generate_link_harness_minimize(work_dir, build_path, rust_build_path, run_test_path, run_all_path, database_dir, user_id, 
                          lib_path, rust_lib_h_path, rust_output_dir, raw_dir, target_dir, target_path, llm_interface, rust_edition)

    # update blocklist
    update_blocklist(rust_lib_h_path, build_rs_path)

    rust_path = lib_path
    
    repair_count = 1
    interface = {
        "trial_id" : trial_id,
        "build_path" : build_path,
        "lib_path" : lib_path,
        "exp_data" : None,
        "rust_build_path" : rust_build_path,
        "target_dir" : raw_dir,
        "list_path" : list_path,
        "meta_dir" : meta_dir,
        "dep_json_path" : dep_json_path,
        "repair_count" : repair_count
    }

    exp_data = {}
    execute_path = f"{work_dir}/execute.sh"
    explore_time = 0
    notes = []

    interface = TransConfig(
        rust_path=rust_path,
        rust_c_path=rust_c_path,
        c_rust_path=c_rust_path,
        target_path=target_path,
        raw_dir=raw_dir,
        rust_output_dir=rust_output_dir,
        select=False,
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
        rust_build_path=rust_build_path,
        run_test_path=run_test_path,
        run_all_path=run_all_path,
        meta_dir=meta_dir,
        div_meta_dir=div_meta_dir,
        dep_json_path=dep_json_path,
        exp_data=exp_data,
        repair_count=repair_count,
        rust_edition=rust_edition,
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
        max_iterations=max_iterations,
        target=target,
        user_id=user_id,
    )

    modified_c_keys = repair_execute('compile', interface)

    # listup configuration
    get_build_rs_config(rust_output_dir, build_config_path)
    
    return build_rs_path, lib_path, toml_path


def record_remaining(finished_units, c_order, components, remained_block_path):
    processed = set()
    for unit in finished_units:
        for comp in unit:
            processed.add((comp['file_path'], comp['start_line'], comp['end_line']))

    lines = []
    for c_id in c_order:
        for comp in components[c_id]:
            if (comp['file_path'], comp['start_line'], comp['end_line']) not in processed:
                lines.append(f"{comp['name']}:{comp['file_path']}:{comp['start_line']}:{comp['end_line']}")

    with open(remained_block_path, 'w') as f:
        f.write('\n'.join(lines) + '\n' if lines else '')



def translate(translation_type, list_path, dep_json_path, meta_dir, div_meta_dir, raw_dir, work_dir, target_dir, database_dir,
                  chat_dir, original_dir, c_code_dir, rust_output_dir, logging_path, count_path, token_path, history_path, moment_path, log_dir, 
                  average, log_file_path, cfg_path, flag_path, build_config_path, rust_edition,
                  run_test_path, run_all_path, build_path, rust_lib_h_path, rust_build_path, target,
                  time_path, map_path, block_path, block_group_path, remained_block_path, progress_queue, max_iterations, llm_interface,
                  rust_c_path, c_rust_path, build_template_path, run_all_template_path, target_path, global_path,
                  is_program_path, resume, previous_block_path, marker, build_rs_path, lib_path, toml_path, trial_id):

    #####################################
    ## Translation
    #####################################

    # turn off warnings
    # turn_off_warning(build_rs_path)

    print("\n========= Translation =========\n")

    c_order, components = get_c_order(block_group_path)
    print(c_order)
    print(len(c_order))

    exp_data = {}
    error = None
    return_path = f"{database_dir}/return.txt"

    if len(c_order) > 0:
        finished = []
        for c_id in c_order: # Add "test": null to all files in the initial state
            one_unit = components[c_id]

            #set_convert_type(moment_path, target, 'divided', llm_choice)
            translate_unit(one_unit, work_dir, raw_dir, target_dir, database_dir, original_dir,
                        target_path, chat_dir, token_path, time_path, progress_queue, log_dir, max_iterations,
                        meta_dir, div_meta_dir, dep_json_path, rust_output_dir, 
                        build_path, lib_path, build_config_path, run_test_path, run_all_path, error, return_path, exp_data,
                        trial_id, rust_build_path, llm_interface, rust_edition,
                        c_rust_path, rust_c_path, is_program_path, target, False
                        ) 
            
            finished.append(one_unit)
            record_remaining(finished, c_order, components, remained_block_path)
    
    else:
        if resume is False:
            raise ValueError(f"{block_group_path} must have at least one element.")
        
        one_unit = []
        translate_unit(one_unit, work_dir, raw_dir, target_dir, database_dir, original_dir,
                        target_path, chat_dir, token_path, time_path, progress_queue, log_dir, max_iterations,
                        meta_dir, div_meta_dir, dep_json_path, rust_output_dir, 
                        build_path, lib_path, build_config_path, run_test_path, run_all_path, error, return_path, exp_data,
                        trial_id, rust_build_path, llm_interface, rust_edition,
                        c_rust_path, rust_c_path, is_program_path, target, True
                        ) 


def handle_paths(dep_json_path):
    paths = [dep_json_path] 
    paths = [f"{MACRO_HOME}/{item}".replace("trans", "macro") for item in paths]
    dep_json_path = paths[0]

    return dep_json_path


def save_target_path(target_path, target_dir):
    dest = os.path.join(target_dir, "actual_targets.txt")
    shutil.copyfile(target_path, dest)


def merge_previous_metadata(previous_meta_dir, meta_dir):
    """
    Carry over `rust_code` fields from previous_meta_dir into meta_dir.
    For each JSON file in previous_meta_dir, find the corresponding file
    in meta_dir and copy `rust_code` from entries that share the same
    `definition` identifier.

    Args:
        previous_meta_dir: Directory containing the previous metadata
        meta_dir: Directory containing the current metadata (merge target)
    """
    print(f"Merging {previous_meta_dir} with {meta_dir}...")

    previous_path = Path(previous_meta_dir)
    meta_path = Path(meta_dir)

    if not previous_path.exists():
        print(f"Previous metadata directory does not exist: {previous_path}")
        return

    previous_json_files = list(previous_path.rglob("*.json"))
    print(f"Found {len(previous_json_files)} JSON files in previous metadata")

    merged_count = 0
    skipped_count = 0

    for prev_json_file in previous_json_files:
        relative = prev_json_file.relative_to(previous_path)
        target_json_file = meta_path / relative

        if not target_json_file.exists():
            print(f"  Skip (no counterpart in meta_dir): {relative}")
            skipped_count += 1
            continue

        prev_data = read_json(prev_json_file)
        target_data = read_json(target_json_file)

        # Build a lookup of previous items keyed by 'definition'.
        # Includes top-level items as well as nested 'uses' and 'components' items
        # so rust_code can be carried over wherever it lived previously.
        prev_lookup = {}
        for prev_item in prev_data:
            if 'definition' in prev_item and 'rust_code' in prev_item:
                prev_lookup[prev_item['definition']] = prev_item['rust_code']

            """
            for callee_item in prev_item.get('uses', []):
                if 'definition' in callee_item and 'rust_code' in callee_item:
                    prev_lookup[callee_item['definition']] = callee_item['rust_code']

            for comp_item in prev_item.get('components', []):
                if 'definition' in comp_item and 'rust_code' in comp_item:
                    prev_lookup[comp_item['definition']] = comp_item['rust_code']

                for callee_item in comp_item.get('uses', []):
                    if 'definition' in callee_item and 'rust_code' in callee_item:
                        prev_lookup[callee_item['definition']] = callee_item['rust_code']
            """

        # Apply rust_code from prev_lookup to matching entries in target_data.
        carried = 0
        for item in target_data:
            if 'definition' in item and item['definition'] in prev_lookup:
                item['rust_code'] = prev_lookup[item['definition']]
                carried += 1

            """
            for callee_item in item.get('uses', []):
                if 'definition' in callee_item and callee_item['definition'] in prev_lookup:
                    callee_item['rust_code'] = prev_lookup[callee_item['definition']]
                    carried += 1

            for comp_item in item.get('components', []):
                if 'definition' in comp_item and comp_item['definition'] in prev_lookup:
                    comp_item['rust_code'] = prev_lookup[comp_item['definition']]
                    carried += 1

                for callee_item in comp_item.get('uses', []):
                    if 'definition' in callee_item and callee_item['definition'] in prev_lookup:
                        callee_item['rust_code'] = prev_lookup[callee_item['definition']]
                        carried += 1
            """

        write_json(target_json_file, target_data)
        merged_count += 1
        print(f"  Merged: {relative} ({carried} rust_code entries carried over)")

    print(f"Merge complete: {merged_count} files merged, {skipped_count} skipped")


def replace_target_dir(previous_target_dir, work_dir):

    previous_path = Path(previous_target_dir)
    target_path = Path(work_dir)

    # Source must exist
    if not previous_path.exists():
        print(f"Error: previous target dir does not exist: {previous_path}")
        sys.exit(1)

    # Remove existing target_dir if present
    if target_path.exists():
        shutil.rmtree(target_path)
    
    # Make sure parent exists, then copy
    target_path.parent.mkdir(parents=True, exist_ok=True)
    shutil.copytree(previous_path, target_path)
    
    print(f"\nReplaced {target_path} with contents of {previous_path}")

    binaries = find_binaries(work_dir)

    for binary in binaries:
        delete_file(binary)


def allrust_compile_main(config): 

    ################################
    #### Configuraion
    ################################

    process_type = config["process_type"]
    user_id = config["user_id"]
    original_dir = config["original_dir"]
    c_code_dir = config["c_code_dir"]
    target_path = config["target_path"]
    block_path = config["block_path"]

    given_meta_dir = config["meta_dir"]
    given_div_meta_dir = config["div_meta_dir"]
    rust_edition = config["rust_edition"]
    llm_choice = config["llm_choice"]
    claude_api_key = config["claude_api_key"]
    azure_endpoint = config["azure_endpoint"]

    resume = config["resume"]
    previous_block_path = config["previous_block_path"]
    previous_meta_dir = config["previous_meta_dir"]
    previous_div_meta_dir = config["previous_div_meta_dir"]
    previous_target_dir = config["previous_target_dir"]

    occupy_path = None 
    build_template_path = f"{MACRO_HOME}/template_rust/template_build.rs"
    build_template_path = f"{TRANS_HOME}/template/template_build.rs"
    run_all_template_path = f"{TRANS_HOME}/template/run_all.sh"

    average = config["average"]

    if check_cargo_modules_installed():
        print("Ready to use cargo modules")
    else:
        raise ValueError("cargo-modules not found")

    # Create PathConfig
    paths = create_path_config(
        user_id=user_id,
        original_dir=original_dir,
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
    original_c_code_dir,
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

    given_block_path, 
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

    if process_type == "trans":

        #******************************************************************
        #*******       Setup       
        #******************************************************************

        log_file_path = set_log(log_dir, llm_choice, target, logging_path, 'convert', DEBUG_LLM)

        print(f"----------- pre-process for {target} started -----------")
        start_time = time.time()

        signal.signal(signal.SIGINT, signal_handler)
        # atexit.register(show_iteration_counts)  # signal.signal(signal.SIGINT, show_iteration_counts)

        llm_interface = LLMInterface(
            project_id=target,
            occupy_path=occupy_path,
            llm_choice=llm_choice,
            full_regions=full_regions,
            llm_model=None,
            temperature=0,
            api_key=None,
            timeout=300,
            output_max=128000, # 4000,
            context_window=1000000,
            history_path=history_path,
            token_path=token_path,
            database_dir=database_dir,
            chat_dir=chat_dir,
            count_path=count_path,
            exp_data={},
        )

        init_prompt_count(count_path)

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

        translation_type = "cs-prompt"
        progress_queue = []
        max_iterations = 5

        if resume is False:
            # initialize
            initialize(translation_type, c_code_dir, rust_output_dir, work_dir, target_dir, raw_dir, database_dir, chat_dir, 
                map_path, logging_path, count_path, token_path, history_path, rust_c_path, c_rust_path, moment_path)

            # Probably not needed in production
            if not os.path.exists(meta_dir) or not os.path.exists(div_meta_dir):
                recreate_directory(meta_dir)
                recreate_directory(div_meta_dir)
                
                clone_directory(given_meta_dir, meta_dir)
                clone_directory(given_div_meta_dir, div_meta_dir)

            denormalize_translation_metadata(meta_dir, os.path.abspath(original_dir), True) #os.path.abspath(target_dir))
            denormalize_translation_metadata(div_meta_dir, os.path.abspath(original_dir), True) #, os.path.abspath(target_dir))

            denormalize_dep_data(dep_json_path, original_dir, os.path.abspath(original_dir)) #, os.path.abspath(target_dir))
            denormalize_target_path(target_path, original_dir, os.path.abspath(original_dir)) #, os.path.abspath(target_dir))

            #denormalize_block_path(block_path, original_dir, os.path.abspath(target_dir)) #denormalize_block_group_path(block_group_path, original_dir, os.path.abspath(target_dir))

            clone_compile_json(os.path.abspath(original_dir), f"{TRANS_HOME}/trans_c_{user_id}", f"{TRANS_HOME}/workspace_{user_id}_{target}")

            """
            error_output, std_output = run_script_wo_log(build_path, 10000, True, None, "build")
            if error_output is not None:
                raise ValueError(f"Faild to run {build_path}")
            """

            """
            compile_dir, compile_json_path = get_compile_json(target_dir)
            find_headers(target_dir, database_dir, dep_json_path, compile_dir, compile_json_path, None)
            """

            #generate_is_program(target_dir, dep_json_path, is_program_path)
            denormalize_block_path(is_program_path, f"{TRANS_HOME}/trans_c_{user_id}/{target}", os.path.abspath(target_dir))
        
        else:
            # merge_previous_metadata(previous_meta_dir, meta_dir)
            merge_previous_metadata(previous_div_meta_dir, div_meta_dir)
            

        #******************************************************************
        #*******       Translation     
        #******************************************************************

        trial_id = set_moment_path(moment_path, average, log_file_path, target)

        if resume is False:
            clang_args_json_path = f"{database_dir}/file_clang_args.json"

            save_target_path(target_path, target_dir)

            if FFI_STRATEGY == "preserve":
                build_rs_path, lib_path, toml_path = setup_build(translation_type, list_path, dep_json_path, meta_dir, div_meta_dir, raw_dir, work_dir, target_dir, database_dir, 
                            chat_dir, original_dir, c_code_dir, rust_output_dir, logging_path, count_path, token_path, history_path, moment_path, log_dir, # , root_dir
                            average, log_file_path, cfg_path, flag_path, build_config_path, rust_edition, # build_list_path, 
                            run_test_path, run_all_path, build_path, rust_lib_h_path, rust_build_path, target, user_id,  # , conds_status_path  # , c_lib_path
                            time_path, map_path, block_path, block_group_path, progress_queue, max_iterations, llm_interface,
                            rust_c_path, c_rust_path, build_template_path, run_all_template_path, target_path, global_path,
                            is_program_path, resume, previous_block_path, marker, trial_id, taken_macros_path,
                            independent_const_build_path, flag_build_path, clang_args_json_path)

            else:
                build_rs_path, lib_path, toml_path = setup_build_minimize(translation_type, list_path, dep_json_path, meta_dir, div_meta_dir, raw_dir, work_dir, target_dir, database_dir, 
                            chat_dir, original_dir, c_code_dir, rust_output_dir, logging_path, count_path, token_path, history_path, moment_path, log_dir, # , root_dir
                            average, log_file_path, cfg_path, flag_path, build_config_path, rust_edition, # build_list_path, 
                            run_test_path, run_all_path, build_path, rust_lib_h_path, rust_build_path, target, user_id, # , conds_status_path  # , c_lib_path
                            time_path, map_path, block_path, block_group_path, progress_queue, max_iterations, llm_interface,
                            rust_c_path, c_rust_path, build_template_path, run_all_template_path, target_path, global_path,
                            is_program_path, resume, previous_block_path, marker, trial_id, taken_macros_path,
                            independent_const_build_path, flag_build_path, clang_args_json_path)    
            
        else:
            # If this is a resume type, consider only the new ones.
            filter_block_path(block_path, previous_block_path)
            
            replace_target_dir(previous_target_dir, work_dir)

            copy_file(f"{TRANS_HOME}/trans_c_{user_id}/{target}/run_test.sh", target_dir)
            copy_file(f"{TRANS_HOME}/trans_c_{user_id}/{target}/c_build.sh", target_dir)

            generate_run_all_path(run_all_path, run_all_template_path, target)

            build_rs_path, lib_path, toml_path = get_existing_lib_paths(work_dir, rust_output_dir)



        remained_block_path = f"{database_dir}/block_remained.txt"
        translate(translation_type, list_path, dep_json_path, meta_dir, div_meta_dir, raw_dir, work_dir, target_dir, database_dir, 
                    chat_dir, original_dir, c_code_dir, rust_output_dir, logging_path, count_path, token_path, history_path, moment_path, log_dir,
                    average, log_file_path, cfg_path, flag_path, build_config_path, rust_edition,
                    run_test_path, run_all_path, build_path, rust_lib_h_path, rust_build_path, target,
                    time_path, map_path, block_path, block_group_path, remained_block_path, progress_queue, max_iterations, llm_interface,
                    rust_c_path, c_rust_path, build_template_path, run_all_template_path, target_path, global_path,
                    is_program_path, resume, previous_block_path, marker, build_rs_path, lib_path, toml_path, trial_id
                    ) 

        output = {
            'work_dir' : work_dir
        }

        if os.path.exists(token_path):
            cost = calc_claude_cost_from_file(token_path)
            print(f"Total cost: ${cost['total_cost_usd']:.2f}")
        print(f"\n\n++++++++++++++= End of translation ({target}) ++++++++++++++=")

        print(f"\nNext action:")
        print(f"\npython3 semantics.py s_repair {TRANS_HOME}/{target_dir}\n")




if __name__ == "__main__":

    #####################################################################
    ##### Input
    #####################################################################
    
    c_code_dir = str(sys.argv[1])
    original_dir = str(sys.argv[2])
    target_path = str(sys.argv[3])
    process_type = str(sys.argv[4])
    meta_dir = str(sys.argv[5])
    div_meta_dir = str(sys.argv[6])
    block_path = str(sys.argv[7])
    resume = str(sys.argv[8])
    rust_edition = "2024" # Rust 2024 edition
    user_id = "0000"

    previous_block_path = None
    previous_meta_dir = None
    previous_div_meta_dir = None
    previous_target_dir = None

    if resume == "on":
        resume = True
        previous_block_path = str(sys.argv[9])
        previous_meta_dir = str(sys.argv[10])
        previous_div_meta_dir = str(sys.argv[11])
        previous_target_dir = str(sys.argv[12])

        for path in [previous_block_path, previous_meta_dir, previous_div_meta_dir, previous_target_dir]:
            if not os.path.exists(path):
                raise FileNotFoundError(f"Path does not exist: {path}")
    else:
        resume = False


    config_data = read_json(f"{CONFIG_PATH}")

    llm_choice = config_data["llm_choice"]
    claude_api_key = config_data["claude_api_key"]
    azure_endpoint = config_data["azure_endpoint"]
    average = config_data["average"]
    TEST_MODE = config_data["test_mode"]
    FFI_STRATEGY = config_data["ffi_strategy"]

    config = {
        "original_dir": original_dir,
        "c_code_dir" : c_code_dir,
        "process_type": process_type,
        "resume" : resume,
        "target_path": target_path,
        "block_path" : block_path,
        "previous_block_path" : previous_block_path,
        "previous_meta_dir" : previous_meta_dir,
        "previous_div_meta_dir" : previous_div_meta_dir,
        "previous_target_dir" : previous_target_dir,
        "meta_dir": meta_dir,
        "div_meta_dir": div_meta_dir,
        "user_id": user_id,
        "rust_edition" : rust_edition,
        "llm_choice": llm_choice,
        "claude_api_key": claude_api_key,
        "azure_endpoint": azure_endpoint,
        "average" : average,
    }

    allrust_compile_main(config)  
