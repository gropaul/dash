import os
import shutil
import sys

def main():

    # go to the explorer-ui directory
    explorer_ui_dir = os.path.join(os.path.dirname(__file__), "..", "explorer-ui")
    # check if the explorer-ui directory exists
    if not os.path.exists(explorer_ui_dir):
        print("explorer-ui directory not found at", explorer_ui_dir)
        print("Please clone the explorer-ui repository in the root directory of the extension repository.")
        sys.exit(1)

    # change the current working directory to explorer-ui
    os.chdir(explorer_ui_dir)

    # check if npm is installed
    if not shutil.which("npm"):
        print("npm not found. Please install npm.")
        sys.exit(1)

    # check if pnpm is installed, if not install it
    if not shutil.which("pnpm"):
        print("pnpm not found. Installing pnpm...")
        os.system("npm install -g pnpm")

    # install dependencies
    print("Installing dependencies...")
    os.system("pnpm install")

    # build the UI
    print("Building UI...")
    os.system("pnpm run build:extension")


if __name__ == "__main__":
    main()
