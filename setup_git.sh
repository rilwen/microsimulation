#!/bin/bash


name=$1
email=$2

if [ "$2" == "" ]; then
	echo "Usage: ${0} <your name> <your email>"
	echo "E.g. ./setup_git.sh "Jane Doe" jane_doe@mail.com"
	exit

fi

# Set your name to what it should appear as in commit messages.
git config user.name "$name"

# Set your email address to what it should appear as in commit messages.
git config user.email "$email"

# Store your GitHub login parameters locally (forever).
git config credential.helper store
