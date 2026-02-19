import subprocess
from pathlib import Path
from shutil import which


def main():
    xmi2mid_app = which("xmi2mid")

    if xmi2mid_app is None:
        print("Could not find xmi2mid in path")
        print("Please install libADLMIDI with xmi2mid support")

        exit(1)

    outputDir = Path("./midis")

    try:
        if not outputDir.exists():
            outputDir.mkdir()
    except IOError as e:
        print(f"Could not create output directory")
        print(f"Reason: {e}")

        exit(1)

    print("Converting XMI -> MID in the current directory")

    curDir = Path(".")

    try:
        xmiFiles = curDir.glob("*.XMI")
    except IOError:
        xmiFiles = []

    for i in xmiFiles:
        print(f"Running xmi2mid on {i}")
        subprocess.run(["xmi2mid", i, "--all", "midis"])


if __name__ == "__main__":
    main()
