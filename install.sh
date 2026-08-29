#!/bin/bash

set -e

echo "========================================="
echo "              A M U N I X"
echo "               installer"
echo "========================================="


# Will use root for installing packages
if command -v sudo >/dev/null 2>&1; then
    ROOT="sudo"
elif command -v doas >/dev/null 2>&1; then
    ROOT="doas"
elif command -v run0 >/dev/null 2>&1; then
    ROOT="run0"
elif command -v pkexec >/dev/null 2>&1; then
    ROOT="pkexec"
else
    ROOT=""
fi

# Checking Dependencies
missing=""

for cmd in gcc make git; do
    if ! command -v "$cmd" >/dev/null 2>&1; then
        missing="$missing $cmd"
    fi
done


if [ -n "$missing" ]; then
    echo "Missing dependencies: $missing"

    INSTALLER=""

    if command -v apt >/dev/null 2>&1; then
        INSTALLER="apt"
    elif command -v dnf >/dev/null 2>&1; then
        INSTALLER="dnf"
    elif command -v pacman >/dev/null 2>&1; then
        INSTALLER="pacman"
    elif command -v brew >/dev/null 2>&1; then
        INSTALLER="brew"
    elif command -v nix >/dev/null 2>&1; then
        INSTALLER="nix"
    elif command -v emerge >/dev/null 2>&1; then
        INSTALLER="emerge"
    fi

    if [ -z "$INSTALLER" ]; then
        echo "No supported package manager detected."
        echo "Please install:$missing"
        exit 127
    fi

    printf "Install dependencies using %s? [Y/n] " "$INSTALLER"
    read -r answer

    case "$answer" in
        [Nn]*) exit 127 ;;
    esac

    case "$INSTALLER" in
        apt)
            $ROOT apt update
            $ROOT apt install -y $missing
            ;;

        dnf)
            $ROOT dnf install -y $missing
            ;;

        pacman)
            $ROOT pacman -S --needed $missing
            ;;

        brew)
            brew install $missing
            ;;

        emerge)
            packages=""

            for dep in $missing; do
                case "$dep" in
                    gcc)
                        packages="$packages sys-devel/gcc"
                        ;;
                    make)
                        packages="$packages sys-devel/gmake"
                        ;;
                    git)
                        packages="$packages dev-vcs/git"
                        ;;
                esac
            done

            $ROOT emerge --ask --oneshot $packages
            ;;

        nix)
            echo "Installing dependencies for Nix..."
            nix profile install \
                nixpkgs#gcc \
                nixpkgs#gnumake \
                nixpkgs#git
            ;;
        
        *)
            echo "Please install: $missing"
            exit 127
            ;;


    esac

fi



echo "Checking Amunix repository..."
# check for Amunix .git instead of cloning again
if [ -d ".git" ] && [ "$(basename "$PWD")" = "Amunix" ]; then
    echo "Inside Amunix repository. Skipping clone."

# do not clone if already cloned
elif [ -d "Amunix" ]; then
    echo "Amunix directory already exists. Skipping clone."
    cd Amunix || exit 0

else
    echo "Cloning Amunix repository..."
    git clone https://github.com/Cryogenicboom/Amunix.git
    cd Amunix || exit 0
fi


# yes please :)
make



# run
if [ -x "./AMUNIX" ]; then
    ./AMUNIX
elif [ -f "./AMUNIX" ]; then
    chmod +x ./AMUNIX
    ./AMUNIX
else
    exit 2
fi

