# godot-cpp template
This repository serves as a quickstart template for GDExtension development with Godot 4.0+.

## Contents
* Preconfigured source files for C++ development of the GDExtension ([src/](./src/))
* An empty Godot project in [demo/](./demo), to test the GDExtension
* godot-cpp as a submodule (`godot-cpp/`)
* GitHub Issues template ([.github/ISSUE_TEMPLATE.yml](./.github/ISSUE_TEMPLATE.yml))
* GitHub CI/CD workflows to publish your library packages when creating a release ([.github/workflows/builds.yml](./.github/workflows/builds.yml))
* An SConstruct file with various functions, such as boilerplate for [Adding documentation](https://docs.godotengine.org/en/stable/tutorials/scripting/cpp/gdextension_docs_system.html)

## Usage - Template

To use this template, log in to GitHub and click the green "Use this template" button at the top of the repository page. This will let you create a copy of this repository with a clean git history.

To get started with your new GDExtension, do the following:

* clone your repository to your local computer
* initialize the godot-cpp git submodule via `git submodule update --init`
* change the name of the compiled library file inside the [SConstruct](./SConstruct) file by modifying the `libname` string.
  * change the paths of the to be loaded library name inside the [demo/bin/example.gdextension](./demo/bin/example.gdextension) file, by replacing `EXTENSION-NAME` with the name you chose for `libname`.
* change the `entry_symbol` string inside [demo/bin/example.gdextension](./demo/bin/example.gdextension) file.
  * rename the `example_library_init` function in [src/register_types.cpp](./src/register_types.cpp) to the same name you chose for `entry_symbol`.
* change the name of the `demo/bin/example.gdextension` file

Now, you can build the project with the following command:

```shell
scons
```

If the build command worked, you can test it with the [demo](./demo) project. Import it into Godot, open it, and launch the main scene. You should see it print the following line in the console:

```
Type: 24
```

### Configuring an IDE
You can develop your own extension with any text editor and by invoking scons on the command line, but if you want to work with an IDE (Integrated Development Environment), you can use a compilation database file called `compile_commands.json`. Most IDEs should automatically identify this file, and self-configure appropriately.
To generate the database file, you can run one of the following commands in the project root directory:
```shell
# Generate compile_commands.json while compiling
scons compiledb=yes

# Generate compile_commands.json without compiling
scons compiledb=yes compile_commands.json
```

## Usage - Actions

This repository comes with a GitHub action that builds the GDExtension for cross-platform use. It triggers automatically for each pushed change. You can find and edit it in [builds.yml](.github/workflows/builds.yml).
After a workflow run is complete, you can find the file `godot-cpp-template.zip` on the `Actions` tab on GitHub.
