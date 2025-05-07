#!/bin/bash
# 206998544 Mati Halamish
# 1 - Source PGN file
# 2 - Destination directory:


src="$1"
#src="$(realpath "$1")"  #2>/dev/null)
#echo "$src"
# echo "$1"
dst_d="$2"


# Check if exactly two arguments are provided and if not print the error
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <source_pgn_file> <destination_directory>"
    exit 1
fi

# check if the file exists 
if ! [ -e "$src" ]; then
    echo "Error: File '$src' does not exist."
    exit 1
fi

#check if the directory exists
if ! [ -d "$dst_d" ]; then
    mkdir "$dst_d"
    echo "Created directory '$dst_d'."
fi


# split the file
filename=$(basename "$src" .pgn) 

while IFS= read -r line; do
    # If the line contains the string "[Event ", start another file
    if [[ "$line" == "[Event "* ]]; then
    i=$((i + 1))
    dst_file="$dst_d/${filename}_$i.pgn" # and once again
    touch "$dst_file" && > "$dst_file"
    echo "Saved game to $dst_file"
    fi
    echo "$line" >> $dst_file
done < "$src"
    
echo "All games have been split and saved to '$dst_d'."

