#!/usr/bin/python

import json
import sys
import string
import os
import re

with open(   sys.argv[1] if len( sys.argv ) > 1 else './compile_commands.json', 'r') as f:
    commands = json.load(f)
    for command in commands:
        directory = command["directory"]
        fl = command["file"]
        if fl[0] != '/':
            absFile = os.path.normpath( directory + "/" + fl )
            command["file"] = absFile

        if command["arguments"]:
            args = command["arguments"]
            cmd = ''
            includePathSpec = False
            for arg in args:
                if includePathSpec:
                   arg = os.path.normpath( directory + "/" + arg[2:] )
                   includePathSpec = False
                if arg == "-I":
                   includePathSpec = True
                cmd += " '" + arg + "'"
            del command["arguments"]
            command["command"] = cmd
        else:
            args = command["command"].split(' ')
            endSkipPattern = None
            for idx, arg in enumerate( args ):
                if arg=="g++":
                    args[idx]="clang++-8"
                if arg=="gcc":
                    args[idx]="clang-8"
                if endSkipPattern:
                    args[idx] = ""
                    if re.search(endSkipPattern, arg):
                        endSkipPattern = None
                if len(arg) >= 3 and arg.startswith('-I') and arg[2] != '/':
                    absVal =  "-I" + os.path.normpath( directory + "/" + arg[2:] )
                    args[idx] = absVal
                # fix knonw macro issues for clang-tidy
                if arg.startswith('-D__declspec(...)') :
                    args[idx] = "-D'__declspec(...)'="
                if arg.startswith('-DLINUXSTUB_C=') :
                    args[idx] = "-DLINUXSTUB_C="
                    endSkipPattern = ".*\);"
                command["command"] = " ".join(args)
    with open( sys.argv[2] if len( sys.argv ) > 2 else './new_compile_commands.json', 'w+' ) as g:
        json.dump( commands, g, indent= 2 ) 

