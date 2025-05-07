#!/bin/bash

#src=a.pgn
src="$1"

if ! [ -e "$src" ]; then
    echo "Error: File '$src' does not exist."
    exit 1
fi

    declare -A board               

initialize_variuables(){
    counter=0    
    local starting_chess_data_line=$(grep -n "^1\." $src | cut -d: -f1 | head -n 1)
     # i know i could do it in one line, but i want it to be readable
    whole_game_data=$(tail -n +$starting_chess_data_line "$src")
    whole_game_data=$(python3 parse_moves.py "\"$whole_game_data\"")        
    num_moves=$(echo $whole_game_data | wc -w)
    data_left=$whole_game_data

    empty="."

    white_pawn="P"
    white_rook="R"
    white_knight="N"
    white_bishop="B"
    white_queen="Q"
    white_king="K"

    black_pawn="p"
    black_rook="r"
    black_knight="n"
    black_bishop="b"
    black_queen="q"
    black_king="k"
}
# Function to initialize the chessboard
initialize_board() {
    counter=0
    data_left=$whole_game_data

    # Fill board with empty squares
    for row in {1..8}; do
        for col in {a..h}; do
            board[$row,$col]="$empty"
        done
    done

    # Place white pieces (uppercase)
    board[1,a]="$white_rook";   board[1,b]="$white_knight"; board[1,c]="$white_bishop"; board[1,d]="$white_queen"
    board[1,e]="$white_king";   board[1,f]="$white_bishop"; board[1,g]="$white_knight"; board[1,h]="$white_rook"

    # Place black pieces (lowercase)
    board[8,a]="$black_rook";   board[8,b]="$black_knight"; board[8,c]="$black_bishop"; board[8,d]="$black_queen"
    board[8,e]="$black_king";   board[8,f]="$black_bishop"; board[8,g]="$black_knight"; board[8,h]="$black_rook"

    for col in {a..h}; do
        board[2,$col]="$white_pawn"  # White pawns
        board[7,$col]="$black_pawn"  # Black pawns
    done
}

# Function to print the chessboard
print_board() {
    echo "Move $counter/$num_moves"
    echo "  a b c d e f g h"
    for row in {8..1}; do
        echo -n "$row "
        for col in {a..h}; do
            echo -n "${board[$row,$col]} "
        done
        echo "$row"
    done
    echo "  a b c d e f g h"
}
print_loop_message() {
    echo -n "Press 'd' to move forward, 'a' to move back, 'w' to go to the start, 's' to go to the end, 'q' to quit:"
    echo ""
}
print_meta_data() {
    echo "Metadata from PGN file:"
    while IFS= read -r line; do
        if [[ "$line" == \[* ]]; then
            echo "$line"
        else
            break
        fi
    done < "$src"
    echo ""
}
#    if [[ "$line" == "[ECO"* ]]; then
#    echo "$line"
#    echo ""
#    fi
#    echo "$line"
#    done < "$src"

go_to_move() {
    local i=$counter
    initialize_board 
    while [[ $i -gt 0 ]]; do
        do_move
        ((i--))
    done
}
check_white_en_passant(){
    if [[ "$src_row" == "5" && "${board[$src_row,$src_col]}" == "$white_pawn" ]]; then
        if [[ "$dst_row" == "6" && "$dst_col" != "$src_col" && "${board[$dst_row,$dst_col]}" == "$empty" ]]; then
            board["$((dst_row - 1)),$dst_col"]="$empty"
        fi
    fi
}
check_black_en_passant(){
    if [[ "$src_row" == "4" && "${board[$src_row,$src_col]}" == "$black_pawn" ]]; then
        if [[ "$dst_row" == "3" && "$dst_col" != "$src_col" && "${board[$dst_row,$dst_col]}" == "$empty" ]]; then
            board["$((dst_row + 1)),$dst_col"]="$empty"
        fi
    fi
}
check_white_castle() {
    if [[ "$src_row" == "1" && "$src_col" == "e" && "${board[$src_row,$src_col]}" == "$white_king" ]]; then
        if [[ "$dst_row" == "1" && "$dst_col" == "g" ]]; then
            board["1,f"]="$white_rook"
            board["1,h"]="$empty"
        elif [[ "$dst_row" == "1" && "$dst_col" == "c" ]]; then
            board["1,d"]="$white_rook"
            board["1,a"]="$empty"
        fi
    fi
}
check_black_castle(){
        # black side castling
    if [[ "$src_row" == "8" && "$src_col" == "e" && "${board[$src_row,$src_col]}" == "$black_king" ]]; then
        if [[ "$dst_row" == "8" && "$dst_col" == "g" ]]; then
            board["8,f"]="$black_rook"
            board["8,h"]="$empty"

        elif [[ "$dst_row" == "8" && "$dst_col" == "c" ]]; then
            board["8,d"]="$black_rook"
            board["8,a"]="$empty"
        fi
    fi

}
check_promotion(){
    if [ -n "$promotion" ]; then
        if [ "$src_row" = "7" ]; then
            promotion=${promotion^^} #change it to upper case since its white
        fi
        board[$src_row,$src_col]=$promotion
    fi
}

do_move() {
    #separate the data into one move and the rest
    read -r move data_left <<< "$data_left" 
    if [ -z "$move" ]; then
#        echo "no more moves left!, do_move function almost made a mistake" 
        return  # Exit the function without affecting the rest of the script
    fi
    src_col=${move:0:1}
    src_row=${move:1:1}
    dst_col=${move:2:1}
    dst_row=${move:3:1}
    promotion=${move:4:1}
#    echo "$move"
    check_promotion
    check_white_castle
    check_black_castle
    check_white_en_passant
    check_black_en_passant

    #do the move
    board["$dst_row,$dst_col"]="${board["$src_row,$src_col"]}"
    board[$src_row,$src_col]=$empty  
    #if it is special move change the board correct 
    ((counter++))
}
#main 
initialize_variuables
print_meta_data
initialize_board
valid_input="true"
#echo $data_left
while true; do
    if [ "$valid_input" = "true" ]; then
        print_board
    fi
    print_loop_message
    read input
    input=$(echo "$input" | xargs)    
    case $input in
        d)
            valid_input="true"  
            if  [ "$counter" -eq "$num_moves" ]; then
                valid_input="false"
                echo "No more moves available."
                continue
            fi
            do_move
            ;;
        a)
            #if its the first move do nothing and print the board
            valid_input="true"
            if [ "$counter" -gt 0 ]; then
                ((counter--))
                go_to_move $counter     
            fi
            ;;
        w)
            valid_input="true"
            initialize_board
            ;;
        s)
            valid_input="true"
            counter=$num_moves
            go_to_move
            ;;
        q)
            echo "Exiting."
            echo "End of game."
            exit 0
            ;;
        *)
            echo "Invalid key pressed: $input"
            valid_input="false"
            ;;
    esac
done