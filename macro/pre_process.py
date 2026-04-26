import os
import networkx as nx
import matplotlib.pyplot as plt
import re
from typing import Dict, List, Tuple, Set, Any
import openai
import copy           
from copy import deepcopy
from collections import defaultdict, deque
import shutil
import subprocess
from functools import reduce
import clang.cindex
#clang.cindex.Config.set_library_file('/usr/lib/llvm-19/lib/libclang.so.1')  # The numbers may change depending on the version
#clang.cindex.Config.set_library_file('/opt/homebrew/opt/llvm/lib/libclang.dylib') # for mac os
from clang.cindex import CursorKind
import tempfile
from pydantic import BaseModel
import concurrent.futures
from concurrent.futures import ThreadPoolExecutor
import time
import anthropic
import json
import base64
import tiktoken
import chardet
from pycparser import c_parser, c_ast
import replicate
from typing import List, Any
from google.generativeai.protos import Content, Part
from anthropic import InternalServerError
import subprocess
from typing import Union, Dict
from clang.cindex import CompilationDatabase, Index
from datetime import datetime, timedelta
from pathlib import Path
import math
from clang.cindex import Index, CursorKind, TokenKind
from clang.cindex import CompilationDatabase, Index
from clang.cindex import CompilationDatabase, CompilationDatabaseError
import stat
from openai import AzureOpenAI
from openai import OpenAI
import sys
import time
import atexit
from pathlib import Path
from typing import Optional
from concurrent.futures import ProcessPoolExecutor
# import google.generativeai as genai  
# from openai import RateLimitError, APIError
# from testGen.main import print_hello

full_regions = []

from utils_api import (
    ## normal
    read_json,
    write_json,
    read_file,
    write_file,
    copy_file,
    delete_file,
    create_file,
    recreate_file,
    create_permissioned_file,
    rename_directory,
    create_directory,
    delete_directory,
    copy_directory,
    clone_directory,
    recreate_directory,
    create_backup_directory,
    run_script,
    run_script_wo_log,
    run_script_pty,
    find_compile_commands_json,
    deduplicate_compile_commands,
    count_file_lines,
    get_timestamp,
    get_all_files,
    get_coverage,
    get_last_directory,
    save_to_output_dir,

    ## translation
    set_log,
    create_path_config,
    extract_all_paths,
    grant_permissions,
    check_permission,
    obtain_metadata,
    get_setting_data,
    get_llm_flag,
    get_lined_specific_code,
    normalize_metafiles,
    normalize_translation_metadata,
    denormalize_translation_metadata,
)

from c_parser_api import (
    analyze_dependencies,
    analyze_call_relationship,
    p_f,
    get_files_list,
    detect_const,
    get_entry_points,
    combine_with_innermost_conditioned_blocks,
    parse_trace,
    setup_macro_without_transforming,
)

from llm_api import (
    LLMInterface,
    init_prompt_count, 
    occupy_llm,
    configure_llm,
    shutdown_llm,
    save_coverage_report,
    get_dir_struct,
    reflect_line_modification,
    ask_llm,
    get_claude_model,
    find_matching_path,
    check_excluded,
    get_lined_code,
    trim_code,
    is_empty_string,
)

MACRO_HOME = "/root/SmartC2Rust/macro"
TRANS_HOME = "/root/SmartC2Rust/trans"
C_PARSER_HOME = "/root/kiso-parser-c"
CONFIG_PATH = "/root/SmartC2Rust/config.json"


MACRO_TRANSFORMATION = False 
DEBUG_LLM = False
TEST_MODE = None 

REPAIR_MAX = 500
guided_line = 20 #10


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
    "reason" : explanatory text for the response (insert here if needed)
}}
"""

def reformat_genifai_testcases(snap_dir, raw_dir, target_dir, database_dir, run_test_path, llm_interface):
    
    output = {}

    files = get_all_files(snap_dir)
    for file_path in files:
        if file_path.endswith(".sh"):
            continue
        
        print(file_path)
        copy_file(file_path, target_dir)
    

    # Collect all .sh files
    sh_contents = []
    for file_path in files:
        if not file_path.endswith(".sh"):
            continue
        
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
            sh_contents.append(f"# --- {os.path.basename(file_path)} ---\n{content}\n")
    
    with open(run_test_path, 'w', encoding='utf-8') as f:
        f.write("#!/bin/bash\n\n")
        f.write("# Combined test cases\n\n")
        f.write("\n".join(sh_contents))
    
    os.chmod(run_test_path, 0o755)

    prompt = [
        "The following shell script combines multiple test shells into one.", # Please semantically separate the testcases and perform the following tasks for each testcase.",
        "Please split them into sequential testcases within a single flat script. Apply the following rules when modifying the shell script:",
        "## Test generation rules:",
        f"  - Do not use subshells '( )' for test execution. Run commands directly and check each command's result.",
        "  - Each testcase must compile to a unique binary name (e.g., test_foo_t1, test_foo_t2) to avoid 'Text file busy' errors when multiple testcases compile the same source file.",
        "  - Do not overwrite a binary that may have been created by a previous testcase.",
        #"  - Each testcase should clean up its own binary after execution.",
        f"  - Do not use 'set -e'. Instead, track failures with a variable.",
        f"  - Initialize 'failed=0' at the top of the script.",
        f"  - At the beginning of each testcase, output \"Test {{test_num}} started\" to standard output.",
        f"  - At the end of each testcase, output \"Test {{test_num}} ended\" to standard output.",
        f"  - When a testcase succeeds, output \"Test {{test_num}} passed\" to standard output.",
        f"  - When a testcase fails, output \"Test {{test_num}} failed\" to standard output and set 'failed=1'.",
        f"  - When a testcase succeeds, output the log to flow_results/test{{{{test_num}}}}_success.log file.",
        f"  - When a testcase fails, output the log to flow_results/test{{{{test_num}}}}_fail.log file.",
        f"  - When a test fails, output the failure message to stderr (e.g., echo \"Test N failed\" >&2).",
        f"  - Please delete '|| true' if it is used. Each test command should fail naturally so that real errors are detected.",
        f"  - At the end of the script, use 'exit $failed' so that the script returns non-zero if any test failed.",
        # f"  - At the beginning of each testcase, output \"Test {{test_num}} started\" to standard output.",
        # f"  - At the end of each testcase, output \"Test {{test_num}} ended\" to standard output.",
        # f"  - If the testcase succeeds, output \"Test {{test_num}} passed\" to standard output.",
        # f"  - If the testcase fails, output \"Test {{test_num}} failed\" to standard output.",
        # f"  - If the testcase succeeds, output the log to flow_results/test{{{{test_num}}}}_success.log file.",
        # f"  - If the testcase fails, output the log to flow_results/test{{{{test_num}}}}_fail.log file.",
        #f"  - Please write the answer in the following JSON format.",
    ]
    prompt.extend(["\n## Response format", "Please write the answer in the following JSON format.",
    ])
    prompt.extend([reformat_response])

    code = read_file(run_test_path)

    prompt.extend([f"## test code ({run_test_path}):",
    ])
    prompt.extend([code])

    prompt.extend(["", "## Directory structure of the translated Rust program:"]) 
    directory_structure = get_dir_struct("translation", target_dir, None)  #rust_output_dir)
    
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
                read_prompt = None # itnitialization

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

            prompt = []
            prompt.extend(["Please continue your response."])
            prompt.extend([
                "Please follow the test rules below when modifying the test script.",
                "\n## Test generation rules:",
                f"  - Do not use subshells '( )' for test execution. Run commands directly and check each command's result.",
                "  - Each testcase must compile to a unique binary name (e.g., test_foo_t1, test_foo_t2) to avoid 'Text file busy' errors when multiple testcases compile the same source file.",
                "  - Do not overwrite a binary that may have been created by a previous testcase.",
                #"  - Each testcase should clean up its own binary after execution.",
                f"  - Do not use 'set -e'. Instead, track failures with a variable.",
                f"  - Initialize 'failed=0' at the top of the script.",
                f"  - At the beginning of each testcase, output \"Test {{test_num}} started\" to standard output.",
                f"  - At the end of each testcase, output \"Test {{test_num}} ended\" to standard output.",
                f"  - When a testcase succeeds, output \"Test {{test_num}} passed\" to standard output.",
                f"  - When a testcase fails, output \"Test {{test_num}} failed\" to standard output and set 'failed=1'.",
                f"  - When a testcase succeeds, output the log to flow_results/test{{{{test_num}}}}_success.log file.",
                f"  - When a testcase fails, output the log to flow_results/test{{{{test_num}}}}_fail.log file.",
                f"  - When a test fails, output the failure message to stderr (e.g., echo \"Test N failed\" >&2).",
                f"  - Please delete '|| true' if it is used. Each test command should fail naturally so that real errors are detected.",
                f"  - At the end of the script, use 'exit $failed' so that the script returns non-zero if any test failed.",
            ])

            prompt.extend(["\n## Response format", "Please write the answer in the following JSON format.",
            ])
            prompt.extend([reformat_response])


        if mode == 'modify_data':
            print(f"In mode: {mode}")
            reflect_line_modification(sum_modified_list, raw_dir, database_dir) # execute_error =  #sum_modified_list.extend(added_list) #if MOD_LINE:
            sum_modified_list = []
                
        #if mode is not None and mode == "modify_data":
        error, std_out = run_script_wo_log(run_test_path, 1000, True, None, "init") # 10 #, progress_queue, iteration_count, max_iterations, log_dir)
        std_out = run_script_pty(run_test_path, 1000) # 60

        if error is None:
            break
    
        else:
            prompt = [
                "The generated test code has the following error. Please fix the test code.",
                # "The following shell script combines multiple test shells into one. Please semantically separate the testcases and perform the following tasks for each testcase.",
                # "## How to modify each testcase:",
                # f"  - At the beginning of each testcase, output \"Test {{test_num}} started\" to standard output.",
                # f"  - At the end of each testcase, output \"Test {{test_num}} ended\" to standard output.",
                # f"  - If the testcase succeeds, output \"Test {{test_num}} passed\" to standard output.",
                # f"  - If the testcase fails, output \"Test {{test_num}} failed\" to standard output.",
                # f"  - If the testcase succeeds, output the log to flow_results/test{{{{test_num}}}}_success.log file.",
                # f"  - If the testcase fails, output the log to flow_results/test{{{{test_num}}}}_fail.log file.",
                # #f"  - Please write the answer in the following JSON format.",
            ]
            prompt.extend([
                "Please follow the test rules below when modifying the test script.",
                "\n## Test generation rules:",
                f"  - Do not use subshells '( )' for test execution. Run commands directly and check each command's result.",
                "  - Each testcase must compile to a unique binary name (e.g., test_foo_t1, test_foo_t2) to avoid 'Text file busy' errors when multiple testcases compile the same source file.",
                "  - Do not overwrite a binary that may have been created by a previous testcase.",
                #"  - Each testcase should clean up its own binary after execution.",
                f"  - Do not use 'set -e'. Instead, track failures with a variable.",
                f"  - Initialize 'failed=0' at the top of the script.",
                f"  - At the beginning of each testcase, output \"Test {{test_num}} started\" to standard output.",
                f"  - At the end of each testcase, output \"Test {{test_num}} ended\" to standard output.",
                f"  - When a testcase succeeds, output \"Test {{test_num}} passed\" to standard output.",
                f"  - When a testcase fails, output \"Test {{test_num}} failed\" to standard output and set 'failed=1'.",
                f"  - When a testcase succeeds, output the log to flow_results/test{{{{test_num}}}}_success.log file.",
                f"  - When a testcase fails, output the log to flow_results/test{{{{test_num}}}}_fail.log file.",
                f"  - When a test fails, output the failure message to stderr (e.g., echo \"Test N failed\" >&2).",
                f"  - Please delete '|| true' if it is used. Each test command should fail naturally so that real errors are detected.",
                f"  - At the end of the script, use 'exit $failed' so that the script returns non-zero if any test failed.",
            ])
            
            prompt.extend(["\n## Response format", "Please write the answer in the following JSON format.", 
            ])
            prompt.extend([reformat_response])

            prompt.extend(["", "## Directory structure of the translated Rust program:"])
            directory_structure = get_dir_struct("translation", target_dir, None)  #rust_output_dir)
            write_file(f"{database_dir}/directry_structure.txt", directory_structure)
            directory_structure = trim_code(f"{database_dir}/directry_structure.txt", directory_structure, 10000)
            prompt.extend([directory_structure, ""])

            prompt.extend(["", "## Error:"])
            prompt.extend([error])



    print("*********** End of reforamt ***********")

    output['run_test_path'] = run_test_path
    return output
            

"""
Add 
-finstrument-functions -g
"""

def reformat_testcases(run_all_path, base_run_path, build_path, raw_dir, target_dir, database_dir, run_test_path, llm_interface):  # snap_dir, 
    
    output = {}

    #config_data = read_json(test_config_path)
    #base_run_path = config_data["base_run_path"]
    #src_files = config_data["src_files"]
    # files = get_all_files(src_files)

    copy_file(base_run_path, target_dir)
    base_run_path = os.path.join(target_dir, os.path.basename(base_run_path))

    with open(run_test_path, 'w', encoding='utf-8') as f:
        f.write("#!/bin/bash\n\n")
        f.write("# Reformed test cases\n\n")
        # f.write("\n".join(sh_contents))

    with open(run_all_path, 'w', encoding='utf-8') as f:
        f.write("#!/bin/bash\n\n")
        f.write("./c_build.sh init\n\n")
        f.write("./run_test.sh\n\n")
        # f.write("\n".join(sh_contents))
    
    os.chmod(run_test_path, 0o755) 
    os.chmod(run_all_path, 0o755)

    prompt = [
        f"The shell script at {base_run_path} is the original test script for the program in {target_dir}.", # Please semantically separate the testcases and perform the following tasks for each testcase.",
        "We want to restructure this test script according to the following rules.",
        f"Please write a new test script that follows the rules below, and modify build-related files as needed so that the new test script can be executed successfully.",
        "",
        "## Modified targets:",
        f"  - Run all script ({run_all_path}): Executes the build script and then the test script in sequence.",
        f"  - Test script ({run_test_path}): Restructure according to the test generation rules below.",
        f"  - Buld-related files ({build_path} or any files): Modify as needed so that the test script compiles and runs correctly.",
        "",
        "## Build modification rules:",
        "  - Each testcase must compile to a unique binary name (e.g., test_foo_t1, test_foo_t2) to avoid 'Text file busy' errors when multiple testcases compile the same source file.",
        "  - Do not overwrite a binary that may have been created by a previous testcase.",
        f"  - Please preserve the original build commands in {build_path} that are required for generating compile_commands.json (e.g., 'bear --' or -DCMAKE_EXPORT_COMPILE_COMMANDS=ON flag.)",
        f"  - If adding new build or compile commands, use them with the same instrumentation tool used in the original build script (e.g., 'bear --' or -DCMAKE_EXPORT_COMPILE_COMMANDS=ON flag.) so that compile_commands.json is updated accordingly.",
        f"  - When using 'bear' to compile additional binaries after the initial 'bear -- make' call, you MUST use 'bear --append --' instead of 'bear --' for all subsequent compilations so that the final compile_commands.json contains all the compilation commands.",
        f"  - Note that the FIRST 'bear' call must NOT use '--append' (use plain 'bear --') because '--append' requires an existing compile_commands.json file to append to.",
        "",
        "## Test generation rules:",
        f"  - Build and execution must be separate in different files.",
        f"        - Build: build-related files ({build_path} or any files)",
        f"        - Test execution: test script ({run_test_path})",
        f"        - Run all: run all script ({run_all_path})",
        f"  - Please create all code considering the directory structure where the files are located.",
        f"  - In particular, please write the shell script considering the directory structure, noting that the shell file execution will be done automatically outside the code in the directory where the shell file is located using ./{{shell_file_name.sh}}.",
        f"  - Do not use subshells '( )' for test execution. Run commands directly and check each command's result.",
        #"  - Each testcase should clean up its own binary after execution.",
        f"  - Do not use 'set -e'. Instead, track failures with a variable.",
        f"  - Initialize 'failed=0' at the top of the script.",
        f"  - At the beginning of each testcase, output \"Test {{test_num}} started\" to standard output.",
        f"  - At the end of each testcase, output \"Test {{test_num}} ended\" to standard output.",
        f"  - When a testcase succeeds, output \"Test {{test_num}} passed\" to standard output.",
        f"  - When a testcase fails, output \"Test {{test_num}} failed\" to standard output and set 'failed=1'.",
        f"  - When a testcase succeeds, output the log to flow_results/test{{{{test_num}}}}_success.log file.",
        f"  - When a testcase fails, output the log to flow_results/test{{{{test_num}}}}_fail.log file.",
        f"  - When a test fails, output the failure message to stderr (e.g., echo \"Test N failed\" >&2).",
        f"  - Please delete '|| true' if it is used. Each test command should fail naturally so that real errors are detected.",
        f"  - At the end of the script, use 'exit $failed' so that the script returns non-zero if any test failed.",
        f"  - When executing each testcase binary, in order to captures function call flow traces for each testcase separately, please prepend the following environment variables to the execution command:",
        f"        LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test{{{{test_num}}}}_trace.log ./binary_name",
        # f"  - At the beginning of each testcase, output \"Test {{test_num}} started\" to standard output.",
        # f"  - At the end of each testcase, output \"Test {{test_num}} ended\" to standard output.",
        # f"  - If the testcase succeeds, output \"Test {{test_num}} passed\" to standard output.",
        # f"  - If the testcase fails, output \"Test {{test_num}} failed\" to standard output.",
        # f"  - If the testcase succeeds, output the log to flow_results/test{{{{test_num}}}}_success.log file.",
        # f"  - If the testcase fails, output the log to flow_results/test{{{{test_num}}}}_fail.log file.",
        #f"  - Please write the answer in the following JSON format.",
    ]
    prompt.extend(["\n## Response format", "Please write the answer in the following JSON format.", 
    ])
    prompt.extend([reformat_response])

    code = read_file(run_test_path)

    prompt.extend([f"## Test code ({run_test_path}):",
    ])
    prompt.extend([code])

    prompt.extend(["", "## Directory structure of the translated Rust program:"]) 
    directory_structure = get_dir_struct("translation", target_dir, None)  #rust_output_dir)
    
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

            prompt = []
            prompt.extend(["Please continue your response."])
            prompt.extend([
                "Please follow the test rules below when modifying the test script.",
                "\n## Test generation rules:",
                f"  - Do not use subshells '( )' for test execution. Run commands directly and check each command's result.",
                "  - Each testcase must compile to a unique binary name (e.g., test_foo_t1, test_foo_t2) to avoid 'Text file busy' errors when multiple testcases compile the same source file.",
                "  - Do not overwrite a binary that may have been created by a previous testcase.",
                #"  - Each testcase should clean up its own binary after execution.",
                f"  - Do not use 'set -e'. Instead, track failures with a variable.",
                f"  - Initialize 'failed=0' at the top of the script.",
                f"  - At the beginning of each testcase, output \"Test {{test_num}} started\" to standard output.",
                f"  - At the end of each testcase, output \"Test {{test_num}} ended\" to standard output.",
                f"  - When a testcase succeeds, output \"Test {{test_num}} passed\" to standard output.",
                f"  - When a testcase fails, output \"Test {{test_num}} failed\" to standard output and set 'failed=1'.",
                f"  - When a testcase succeeds, output the log to flow_results/test{{{{test_num}}}}_success.log file.",
                f"  - When a testcase fails, output the log to flow_results/test{{{{test_num}}}}_fail.log file.",
                f"  - When a test fails, output the failure message to stderr (e.g., echo \"Test N failed\" >&2).",
                f"  - Please delete '|| true' if it is used. Each test command should fail naturally so that real errors are detected.",
                f"  - At the end of the script, use 'exit $failed' so that the script returns non-zero if any test failed.",
            ])

            prompt.extend(["\n## Response format", "Please write the answer in the following JSON format.",
            ])
            prompt.extend([reformat_response])


        if mode == 'modify_data':
            print(f"In mode: {mode}")
            reflect_line_modification(sum_modified_list, raw_dir, database_dir) # execute_error =  #sum_modified_list.extend(added_list) #if MOD_LINE:
            sum_modified_list = []
                
        #if mode is not None and mode == "modify_data":
        error, std_out = run_script_wo_log(run_all_path, 1000, True, None, "init")  # 10 #, progress_queue, iteration_count, max_iterations, log_dir)
        std_out = run_script_pty(run_test_path, 1000) # 60

        if error is None:
            break
    
        else:
            prompt = [
                "The generated test code has the following error. Please fix the test code.",
                # "The following shell script combines multiple test shells into one. Please semantically separate the testcases and perform the following tasks for each testcase.",
                # "## How to modify each testcase:",
                # f"  - At the beginning of each testcase, output \"Test {{test_num}} started\" to standard output.",
                # f"  - At the end of each testcase, output \"Test {{test_num}} ended\" to standard output.",
                # f"  - If the testcase succeeds, output \"Test {{test_num}} passed\" to standard output.",
                # f"  - If the testcase fails, output \"Test {{test_num}} failed\" to standard output.",
                # f"  - If the testcase succeeds, output the log to flow_results/test{{{{test_num}}}}_success.log file.",
                # f"  - If the testcase fails, output the log to flow_results/test{{{{test_num}}}}_fail.log file.",
                # #f"  - Please write the answer in the following JSON format.",
            ]
            prompt.extend([
                "Please follow the test rules below when modifying the test script.",
                "\n## Test generation rules:",
                f"  - Do not use subshells '( )' for test execution. Run commands directly and check each command's result.",
                "  - Each testcase must compile to a unique binary name (e.g., test_foo_t1, test_foo_t2) to avoid 'Text file busy' errors when multiple testcases compile the same source file.",
                "  - Do not overwrite a binary that may have been created by a previous testcase.",
                #"  - Each testcase should clean up its own binary after execution.",
                f"  - Do not use 'set -e'. Instead, track failures with a variable.",
                f"  - Initialize 'failed=0' at the top of the script.",
                f"  - At the beginning of each testcase, output \"Test {{test_num}} started\" to standard output.",
                f"  - At the end of each testcase, output \"Test {{test_num}} ended\" to standard output.",
                f"  - When a testcase succeeds, output \"Test {{test_num}} passed\" to standard output.",
                f"  - When a testcase fails, output \"Test {{test_num}} failed\" to standard output and set 'failed=1'.",
                f"  - When a testcase succeeds, output the log to flow_results/test{{{{test_num}}}}_success.log file.",
                f"  - When a testcase fails, output the log to flow_results/test{{{{test_num}}}}_fail.log file.",
                f"  - When a test fails, output the failure message to stderr (e.g., echo \"Test N failed\" >&2).",
                f"  - Please delete '|| true' if it is used. Each test command should fail naturally so that real errors are detected.",
                f"  - At the end of the script, use 'exit $failed' so that the script returns non-zero if any test failed.",
            ])
            
            prompt.extend(["\n## Response format", "Please write the answer in the following JSON format.",
            ])
            prompt.extend([reformat_response])

            prompt.extend(["", "## Directory structure of the translated Rust program:"])
            directory_structure = get_dir_struct("translation", target_dir, None)  #rust_output_dir)
            write_file(f"{database_dir}/directry_structure.txt", directory_structure)
            directory_structure = trim_code(f"{database_dir}/directry_structure.txt", directory_structure, 10000)
            prompt.extend([directory_structure, ""])

            prompt.extend(["", "## Error:"])
            prompt.extend([error])

    print("*********** End of reforamt ***********")

    output['run_test_path'] = run_test_path
    return output
            



def _is_different_file(actual_path: Path, referenced_name: str) -> bool:
    """
    Determines whether the filename referenced by #line differs from the actual file.
    """
    if referenced_name.startswith("<"):
        return False
    
    actual_name = actual_path.name
    referenced_stem = Path(referenced_name).name
    
    return actual_name != referenced_stem



def detect_auto_generated_files(project_dir: str) -> dict[str, list[dict]]:
    """
    Scans C source files within the project directory
    and detects files considered auto-generated based on #line directives.
    
    Returns:
        dict: {auto-generated file path: [list of detected #line directive information]}
    """
    line_directive_pattern = re.compile(
        r'^\s*#\s*(?:line\s+)?(\d+)\s+"([^"]+)"', re.MULTILINE
    )
    
    generated_files: dict[str, list[dict]] = {}
    project_path = Path(project_dir).resolve()
    
    for source_file in project_path.rglob("*"):
        if source_file.suffix not in (".c", ".h", ".cc", ".cpp", ".cxx"):
            continue
        
        try:
            content = source_file.read_text(encoding="utf-8", errors="replace")
        except (OSError, PermissionError):
            continue
        
        directives = []
        for line_no, line in enumerate(content.split("\n"), start=1):
            match = line_directive_pattern.match(line)
            if match:
                referenced_file = match.group(2)
                referenced_line = int(match.group(1))
                
                if _is_different_file(source_file, referenced_file):
                    directives.append({
                        "actual_line": line_no,
                        "referenced_file": referenced_file,
                        "referenced_line": referenced_line,
                        "raw": line.strip(),
                    })
        
        if directives:
            generated_files[str(source_file)] = directives
    
    return generated_files


def find_log_paths(results_dir):
    """Find all trace log files in the results directory (recursive)"""
    log_dir = os.path.abspath(results_dir)
    log_paths = sorted(
        os.path.join(root, f)
        for root, _, files in os.walk(log_dir)
        for f in files
        if f.endswith("_trace.log")
    )
    return log_dir, log_paths


def get_test_number(log_path):
    """Extract test number from trace log filename (e.g. test1_trace.log -> 1)"""
    basename = os.path.basename(log_path)
    match = re.search(r"test(\d+)_trace\.log", basename)
    if match:
        return match.group(1)
    raise ValueError(f"Could not extract test number from {basename}")


def find_base_dir(original_dir):
    """Find the directory containing c_build.sh by searching up from original_dir"""
    current = os.path.abspath(original_dir)
    while current != os.path.dirname(current):
        if os.path.exists(os.path.join(current, "c_build.sh")):
            return current
        current = os.path.dirname(current)
    raise FileNotFoundError(f"c_build.sh not found above {original_dir}")


def set_golden_dir(original_dir):
    # Find flow_results path

    log_dir, log_paths = find_log_paths(original_dir)
    print(original_dir)
    print(log_dir)
    print(log_paths)

    #upper_dir = os.path.dirname(log_dir)
    golden_dir = os.path.join(original_dir, "golden")
    print(golden_dir)

    os.makedirs(golden_dir, exist_ok=True)
    base_dir = find_base_dir(original_dir)
    print(base_dir)

    """
    for log_path in log_paths:
        test_number = get_test_number(log_path)
        print(test_number)
    
        golden_flow_path = f"{golden_dir}/test{test_number}_golden_flow.txt"

        parse_trace(log_path, base_dir, golden_flow_path, False) # binary_path: str, 
    """

    tasks = []
    for log_path in log_paths:
        test_number = get_test_number(log_path)
        print(test_number)
        golden_flow_path = f"{golden_dir}/test{test_number}_golden_flow.txt"
        tasks.append((log_path, base_dir, golden_flow_path, False))

    #with ProcessPoolExecutor(max_workers=4) as executor:
    with ProcessPoolExecutor() as executor:
        futures = {
            executor.submit(parse_trace, *args): args[2]
            for args in tasks
        }
        for future in futures:
            try:
                future.result()
            except Exception as e:
                print(f"ERROR: {futures[future]}: {e}")


def initialize(target_dir, meta_dir, database_dir, dep_json_path): 

    # initialize directories and files
    #delete_directory(raw_dir) # Do not delete here, otherwise parallel execution of multiple programs is not possible
    delete_directory(target_dir)
    delete_directory(meta_dir)

    database_json_path = f"{database_dir}/compile_commands.json"
    delete_file(database_json_path)

    # delete_file(f'{database_dir}/m_conds.json')
    # delete_file(f'{database_dir}/m_grep.json')

    # delete_file(f"{database_dir}/dep_user.json")
    # delete_file(f'{database_dir}/macro_func.txt')

    #delete_file(token_path)
    #delete_file("token_macro.json")

    #delete_file('find_defines.c') # Not verified
    #delete_file('find_if.c') # Not verified

    # create new directories
    if not DEBUG_LLM:
        # delete_directory(database_dir)
        # create_directory(database_dir)

        # list of files to keep
        keep_files = {"answer.json"}
        
        if os.path.exists(database_dir):
            for item in os.listdir(database_dir):
                item_path = os.path.join(database_dir, item)
                
                # delete items not included in keep_files
                if item not in keep_files:
                    if os.path.isfile(item_path):
                        os.remove(item_path)
                        print(f"Deleted: {item_path}")
                    elif os.path.isdir(item_path):
                        shutil.rmtree(item_path)
                        print(f"Deleted directory: {item_path}")
        else:
            create_directory(database_dir)
    
    # delete_directory(root_dir)
    # delete_directory("preprocessed_output")
    #delete_file(macro_list_path)
    #delete_file(macro_path)
    #delete_file(initial_macro_path)

    #delete_file(initial_list_path)
    #delete_file(all_macro_path)

    # write_json(f"{database_dir}/pro_functions.json", {})
    # write_json(f"{database_dir}/pro_data_type.json", {})
    # write_json(f"{database_dir}/pro_global_var.json", {})
    # write_json(f"{database_dir}/pro_macro.json", {})

    # write_json(f"{database_dir}/pro_pro_functions.json", {})
    # write_json(f"{database_dir}/pro_pro_data_type.json", {})
    # write_json(f"{database_dir}/pro_pro_global_var.json", {})
    # write_json(f"{database_dir}/pro_pro_macro.json", {})

    # write_json("pro_pro_functions.json", [])
    # write_json("pro_pro_data_type.json", [])
    # write_json("pro_pro_global_var.json", [])
    # write_json("pro_pro_macro.json", [])



def macro_main(config):

    ############################################
    ##### Configuration
    ############################################

    process_type = config["process_type"]
    user_id = config["user_id"]
    original_dir = config["original_dir"]
    given_test_path = config["run_test_path"]
    
    target_path = config["target_path"]
    llm_choice = config["llm_choice"]
    claude_api_key = config["claude_api_key"]
    azure_endpoint = config["azure_endpoint"]
    out_meta_dir = config["out_meta_dir"]

    macro_finder = f"{MACRO_HOME}/macro_finder/build/macro-finder"
    occupy_path = None

    claude_model = get_claude_model(llm_choice)

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

    output = {}
    if process_type == "reformat":

        reformat_type = config["reformat_type"]

        print("************ reformat process started ************")
        # initialize
        initialize(target_dir, meta_dir, database_dir, dep_json_path)

        print(f"original_dir: {original_dir}") 
        copy_directory(original_dir, raw_dir)
        #grant_permissions(target_dir) # This sometimes prevents compile_commands.json from being displayed: ex tiff-4.3.0

        exp_data = {}
        llm_interface = LLMInterface(
            project_id=target,
            occupy_path=occupy_path,
            llm_choice=llm_choice,
            full_regions=full_regions,
            llm_model=claude_model,
            output_max=128000, # 4000,
            context_window=1000000,
            temperature=0,
            api_key=None,
            timeout=300,
            history_path=f"{database_dir}/history.json",
            token_path=token_path, #f"{database_dir}/tokens.json",
            database_dir=f"{database_dir}",
            chat_dir=f"{chat_dir}",
            count_path=f"{database_dir}/count.json",
            exp_data=exp_data
        )

        recreate_directory(chat_dir)
        recreate_file(history_path)

        grant_permissions(target_dir) 
        copy_file(run_test_path, target_dir)

        run_all_path = f"{target_dir}/run_all.sh"
        
        llm_model = get_claude_model(llm_choice)
        llm_interface = configure_llm(
            llm_interface,
            claude_api_key,
            azure_endpoint,
            llm_model
        )
        if TEST_MODE:
            llm_interface = occupy_llm(llm_interface)
            atexit.register(lambda: shutdown_llm(llm_interface))

        run_script(build_path, 100000, True, None, "init", None, 0, 0, None, None)

        if reformat_type == "genifai":
            reformat_genifai_testcases(snap_dir, raw_dir, target_dir, database_dir, run_test_path, llm_interface)
        
        else:
            base_test_path = config["base_test_path"]
            reformat_testcases(run_all_path, base_test_path, build_path, raw_dir, target_dir, database_dir, run_test_path, llm_interface)


        output = {
            'run_test_path' : run_test_path
        }
    
        print("\n************ End of reformat process ************")
        print(f"\nNext action->")
        print(f"python3 pre_process.py {MACRO_HOME}/trans_re_0000/{target} golden\n") # {os.path.abspath(meta_dir)} {os.path.abspath(div_meta_dir)} {os.path.abspath(compile_dir)}")
        
    elif process_type == "golden":

        print(original_dir)
        #print(target_path)
        set_golden_dir(original_dir)

        print("\n************ End of golden process ************")
    
        print(f"\nNext action->")
        print(f"python3 pre_process.py {MACRO_HOME}/trans_re_0000/{target} macro off {MACRO_HOME}/trans_re_0000/{target}/run_test.sh /root/SmartC2Rust/benchmark/{target}/targets.txt\n") # {os.path.abspath(meta_dir)} {os.path.abspath(div_meta_dir)} {os.path.abspath(compile_dir)}")

    elif process_type == "macro":

        #####################################
        ##### Program start
        #####################################

        print("************ pre-process started ************")
        start_time = time.time()

        # copy_directory(original_dir, ".")
        # set_log(log_dir, llm_choice, target, logging_path, 'pre_processing')

        # initialize
        initialize(target_dir, meta_dir, database_dir, dep_json_path) #,  # , flag_json_path 
                   #macro_list_path, macro_path, all_macro_path)  # , initial_macro_path

        # copy the target directory
        check_permission(original_dir)

        copy_directory(original_dir, raw_dir)
        #grant_permissions(raw_dir)  # If this is enabled, compile_commands.json may occasionally not appear (e.g., tiff-4.3.0)

        copy_file(given_test_path, run_test_path)

        print(f"original_dir: {original_dir}") 
        print(raw_dir)
        print(target_dir)
        print(run_test_path)
        print(build_path)

        exp_data = {}
        llm_interface = LLMInterface(
            project_id=target,
            occupy_path=occupy_path,
            llm_choice=llm_choice,
            full_regions=full_regions,
            llm_model=claude_model,
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
            exp_data=exp_data
        )

        recreate_directory(chat_dir)
        grant_permissions(target_dir) # If this is enabled, compile_commands.json may occasionally not appear (e.g., tiff-4.3.0)
        
        llm_interface = configure_llm(
            llm_interface,
            claude_api_key,
            azure_endpoint,
            claude_model
        )

        if TEST_MODE:
            llm_interface = occupy_llm(llm_interface)
            atexit.register(lambda: shutdown_llm(llm_interface))

        # atexit.register(lambda: cleanup(work_dir, meta_dir, snap_dir))
        
        generated = detect_auto_generated_files(target_dir)
        if not generated:
            print("No auto-generated files detected.")
        else:
            for file_path, directives in generated.items():
                origins = set(d["referenced_file"] for d in directives)
                print(f"  {file_path} ← generated from {', '.join(origins)}")
            #raise ValueError(f"Detected {len(generated)} auto-generated file(s):\n")
                
        # sys_macros_path = f"{database_dir}/sys_macros.json"
        llm_on = False

        if MACRO_TRANSFORMATION is True:
            print("TBA")
        
        else:
            setup_macro_without_transforming(llm_on, macro_finder, target_dir, database_dir, meta_dir, div_meta_dir, build_path, cfg_path, target_path, marker, 
                                    list_path, dep_json_path, custom_headers_dir, custom_json_path, custom_header_path, # , build_rs_path  # run_test_path, call_path  #picked_path, macro_list_path, macro_path, all_macro_path, # classified_path, # defined_path, undefined_path, cmd_line_path, 
                                    llm_choice, llm_interface, token_path, chat_dir, all_macros_path, taken_macros_path, 
                                    all_directive_path, taken_directive_path, is_program_path, global_path,
                                    guards_path, guarded_macros_path, independent_path, flag_path, const_path, conflict_path # , sys_macros_path
                                    )


        normalize_metafiles(meta_dir, raw_dir, all_macros_path, taken_macros_path, guards_path)        
        
        normalize_translation_metadata(meta_dir, raw_dir)
        normalize_translation_metadata(div_meta_dir, raw_dir)

        output = {
            'c_code' : target_dir,
            'all_macros_path' : all_macros_path,
            'taken_macros_path' : taken_macros_path,
            #'sys_macros_path' : sys_macros_path,
            'guards_path' : guards_path,
            'cfg_path' : cfg_path,
        }

        compile_dir = find_compile_commands_json(target_dir)

        # analyze_macros_llm(target_dir, c_run_path, picked_path, macro_path, call_path, classified_path, defined_path, undefined_path, cmd_line_path)
        
        print("\n************ End of macro analysis ************\n")        
        print("\ncd ~/SmartC2Rust/trans")
        print(f"python3 pre_process.py {MACRO_HOME}/trans_c_0000/{target} meta {target_path} {os.path.abspath(meta_dir)} {os.path.abspath(div_meta_dir)} {os.path.abspath(compile_dir)}")

    save_to_output_dir(output, output_dir)
    return output, output_dir



if __name__ == "__main__": 

    # python activate
    # https://qiita.com/yokoto/items/6d9e7d0ee440b3d147ac
    # python3 -m venv .python/venv
    # source .python/venv/bin/activat

    #####################################################################
    ##### Input
    #####################################################################
    
    original_dir = str(sys.argv[1])
    process_type = str(sys.argv[2])

    given_test_path = None
    target_path = None
    reformat_type = None
    snap_dir = None
    base_test_path = None

    if process_type == "reformat":
        llm_on = "on"
        reformat_type = str(sys.argv[3])
        if reformat_type == "genifai":
            snap_dir = str(sys.argv[4]) 
        else:
            base_test_path = str(sys.argv[4])

    # """
    # elif process_type == "golden":
    #     target_path = str(sys.argv[3])
    # """

    elif process_type == "macro":
        llm_on = str(sys.argv[3]) # process_type = "meta"
        given_test_path = str(sys.argv[4])
        target_path = str(sys.argv[5])
        #target_path = f"{MACRO_HOME}/benchmark/mini2/targets.txt" # Should change here
    
    user_id = "0000"

    config_path = f"{CONFIG_PATH}"  # This is being affected
    config_data = read_json(config_path)
    #target_path = f"{MACRO_HOME}/benchmark/{target}/targets_actual.txt" # Should change this
    llm_choice = config_data["llm_choice"]
    claude_api_key = config_data["claude_api_key"]
    azure_endpoint = config_data["azure_endpoint"] 
    TEST_MODE = config_data["test_mode"] 

    config = {
        "process_type": process_type,
        "reformat_type" : reformat_type,
        "user_id": user_id,
        "original_dir": original_dir,
        "run_test_path": given_test_path,
        "base_test_path" : base_test_path,
        "target_path": target_path,
        "llm_choice": llm_choice,
        "claude_api_key": claude_api_key,
        "azure_endpoint": azure_endpoint,
        "out_meta_dir": None,
    }

    output, output_dir = macro_main(config)
    
    if process_type == "reformat":  # print(f"Saved at {output_dir}")
        original_run_test_path = f"{original_dir}/run_test.sh"
        copy_file(output['run_test_path'], original_dir)
        # print(f"Saved at {original_run_test_path}")

