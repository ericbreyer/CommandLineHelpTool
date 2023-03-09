# Command Line Help Tool (Bash Terminal Command Helper)

Leveraging openai to create a command line command helper.

## Usage
```btch [-s <shell>] [-n <number>] [-k] "<command description>"```
optional arguemnts:
- `-s <shell>` : the shell you are using (defualts to bash)
- `-n <number>` : number of results to generate (defaults to one, only use if the result from one is insufficient and you want some varaibility)
- `-k` : force reentering of the openai api key (key gets cached after each run)

Note: If you are running the command for the first time (or it can not find the key file), it will ask for your openai api key regardless of the `-k` flag.

## Instalation
### Build from source for macOS (will probably work on other *nix systems but is untested)
1. Clone this repository
2. `cd` into the directoy
3. run `make depends` (or otherwise install libcurl)
4. run `make` on the command line
5. run `make install`
