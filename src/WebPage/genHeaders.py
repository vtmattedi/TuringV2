# Reads Files from 'WebPageFolder' and turns them into a raw string literal for c/c++.
# Each file will be converted into a .h file in the output folder with a const char* 
# with the contents of the source file. Afterwards, a file named webfiles.h is created
# in the output folder. This file only includes all other .h files in the output folder
# so it is easier to just add '#include "{output folder}/webpage.h" ' to the code and
# then have access to all the files in 'WebPageFolder'.
# Useful when designing websites for esp32 and other microcontrollers. You can directly
# edit the files in your preferred IDE and the changes will be reflected on the .h files at build.
# Add this script to extra_scripts in plataformio.ini with the 'pre:' prefix to automatically update the
# webpage files whenever you build your project.
#   in platformio.ini:
#       ```...
#          extra_scripts = pre:{this_file_folder}/{this_file_name}.py
#          ...
#       ```
#  TODO: Change from a global list to read the files in the output folder.
#  TODO: ADD custom variables to the environment so that we can customize them to replace some options such as
#  INCLUDE_SUBFOLDERS and USE_STATIC_CONSTANTS and WebPageFolder
#

import datetime
import os
Import("env")
import re

DEBUG_PDIR = False          # Prints the 'WebPageFolder' and the Output folder.
DEBUG_FILEDIR = False       # Prints the filename of all files been looped
DEBUG_HEADERNAME = False    # Prints the filename for .h 
DEBUG_SAVEDFILES = False    # Prints files generated, that will be added to webpage.h
STOP_BUILD = False          # Set to True to raise an exception after this script is done. useful to see the debugs.
INCLUDE_SUBFOLDERS = True   # Set to True to loop through subfolders of the 'WebPageFolder'
USE_STATIC_CONSTANTS = True # if True, will try to find definitions for words inside braces and replace them
                            # I.E. {device_name} will be replace if #define device_name is found in any of the
                            # FILES_WITH_STATIC_CONSTANTS

FILES_WITH_STATIC_CONSTANTS = [env.subst("$PROJECT_DIR") + os.sep + "src" + os.sep + "main.cpp", 
                            env.subst("$PROJECT_DIR") + os.sep + "include" + os.sep + "Version.h"]

WebPageFolder = env.subst("$PROJECT_DIR") + os.sep + "src" + os.sep + "WebPage" + os.sep
DEST_DIR = os.path.join(WebPageFolder, "Output")

def getValueFromFile(files,value):
    found = False
    foundValue = ' '
    for file in files:
        if (os.path.isfile(file)):
            input_string = ''
            with open(file, 'r') as f:
                input_string = f.read()
            # Define the regular expression pattern to match DEVICE_NAME followed by the target word
            pattern = rf'#define {re.escape(value)}\s*"([^"]+)"'
            # Use re.search() to find the pattern in the input string
            match = re.search(pattern, input_string, re.I)

            # Check if the pattern is found and get the word after "device_name ="
            if match:
                foundValue = match.group(1)
                #print(f"Word after '{value}' is:", foundValue)
                found = True
            #else:
                #print("No match found for 'device_name =' in the input string.")
    return found, foundValue

if DEBUG_PDIR:
    print(f"Web Page folder : '{WebPageFolder}'")
    print(f"Output folder : '{DEST_DIR}'")
saved_files_names = []

def gen_headers(src_dir, dest_dir, tree = 0):
    #ignore Output folder
    if src_dir.endswith(dest_dir):
        return
    # Check if source directory exists
    if not os.path.exists(src_dir):
        print(f"Source directory '{src_dir}' does not exist.")
        return
    
    # Check if the destination directory exists, create if not
    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)
    
    # Iterate through files in the source directory
    for filename in os.listdir(src_dir):
        src_file = os.path.join(src_dir, filename)
        if DEBUG_FILEDIR:
            print(f'Current file: {src_file}')
        # Check if it's a file
        # Ignore Python scripts
        if os.path.isfile(src_file) and not src_file.endswith(".py"):
            with open(src_file, 'r') as f:
                file_content = f.read()
                if (USE_STATIC_CONSTANTS):
                    #Search for words inside braces
                    pattern = r'\{([^}\n]+)\}'
                    matches = re.findall(pattern, file_content)
                    if matches:
                        for match in matches:
                            #foreach value in braces, search on the replace file for the definition
                            result = getValueFromFile(FILES_WITH_STATIC_CONSTANTS,match)
                            #result [0] -> Found, result[1] -> value
                            #if found a definition replace the value with the definition
                            if (result[0]):
                                file_content = file_content.replace("{"+ match + "}", result[1])
                                print('replacing',match,' -> ',result)
            # Create the C-style string constant name
            c_string_filename = f'const char *{filename.replace(".", "_")} = R"==({file_content})==";'
            # Write the string to a header file
            header_filename = os.path.join(dest_dir, f'{filename.replace(".", "_")}.h')
            if os.path.isfile(header_filename):
                os.remove(header_filename)
            with open(header_filename, 'w') as f:
                f.write(c_string_filename)
            if(DEBUG_HEADERNAME):
                print(f"Created header file: '{header_filename}'")
            saved_files_names.append(header_filename)
        #loop through folders
        elif os.path.isdir(src_file) and INCLUDE_SUBFOLDERS:
            gen_headers(src_file, dest_dir, tree+1)
    
def gen_headers_super(saved_files_names,dest_dir):
    super_path = os.path.join(dest_dir, f'webfiles.h')
    with open(super_path, 'w+') as file:
        #print(saved_files_names)
        file.write("#pragma once\n")
        for filename in saved_files_names:
            #print(filename)
            file.write(f'#include <{filename}>\n')
        file.close()

# Generates headers and saved_files_names list
gen_headers(WebPageFolder,DEST_DIR)

# Debug files stored in saved_files_names
if DEBUG_SAVEDFILES:
    for i, file in enumerate(saved_files_names):
        print(f"[{i}] - '{file}'.")
# Generates DEST_DIR/webfiles.h that includes all files in DEST_DIR
gen_headers_super(saved_files_names, DEST_DIR)
# Stops build process. Useful for debug
if STOP_BUILD:
    raise Exception("stop here")