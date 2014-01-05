# cocoa

## How to add a new project
1. Click "Build Settings"
2. Search it for flag.
3. Add -lz to "Other Linker Flags"
4. Select "Build Phases"
5. Move a ggo file from "Copy Files" to "Compile Sources"
6. Select "Build Rules"
7. Select a menu "Editor -> Add Build Rules"
8. Copy the following two lines and paste them to the rule.
cd "$INPUT_FILE_DIR"
/usr/local/bin/gengetopt -i "$INPUT_FILE_NAME" --output-dir="$DERIVED_SOURCES_DIR" --file-name=cmdline --unamed-opts --func-name=my_cmdline_parser --include-getopt
9. Type in *.ggo in the Process input box.
10. Add the following two folders from the alder cocoa projects: file and
bstrlib.
11. Create a folder named after the project name without "alder-"
12. Move the main source and header files to the newly created folder.

.Alder command line
- Add a new rule
- Move alder-something.ggo to the source code
- Add other packages to the given project

