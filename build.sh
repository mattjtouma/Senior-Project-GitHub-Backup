#!/bin/bash

echo "building..."

# Build the cgi script
g++ controlAD9833.cpp -o controlAD9833.cgi -lcgicc

# Copy it to the Apache2 server location (root directory)
sudo cp controlAD9833.cgi /usr/lib/cgi-bin/

# Modify permissions
sudo chmod +s /usr/lib/cgi-bin/controlAD9833.cgi

echo "done building."


# -------- GitHub --------

# Temporarily remove the backup folder
echo "removing backup foder"
sudo rm -r Senior-Project-GitHub-Backup

# Clone the repository from Github
echo "cloning repository"
git clone https://github.com/mattjtouma/Senior-Project-GitHub-Backup.git

# Copy the files to the backup folder except the bakcup folder itself
echo "copying all files"
#cp -a ./ Senior-Project-GitHub-Backup
rsync -av ./ Senior-Project-GitHub-Backup/ --exclude=Senior-Project-GitHub-Backup

echo "changing directory"
cd Senior-Project-GitHub-Backup/

# No need to backup the backup
echo "removing the backup file from the staging area"
#echo "Senior-Project-GitHub-Backup" > .gitignore
git rm -r Senior-Project-GitHub-Backup/

# Add all files in the curren working directory to the staging area
echo "adding files"
git add .


git config --global user.name "mattjtouma"
git config --global user.email "mattjtouma@gmail.com"
git commit -m "Backing up Senior-Project files"

git config --global push.default simple
git push

echo "done"
