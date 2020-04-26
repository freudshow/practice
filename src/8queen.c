#include <stdio.h>

static int board[8] = {};
int board_size = sizeof(board)/sizeof(board[0]);

int check(int *board,int row){
    int i = 0;
    while(i < row){
        if(board[i] == board[row] || row - i == board[row] - board[i] || row - i == board[i] - board[row]){
            return 0;
        }
        i++;
    }
    // printf("board[%d]: %d\n",row,board[row]);
    return 1;
}

void print_board(int *board){
    int i;
    int size = board_size;
    for(i=0;i<size;i++){
        printf("%d,",board[i]);
    }
    printf("\n");
    i = 0;
    while (i < size){
        int j;
        for (j=0;j<size;j++){
            if(j == board[i]){
                printf("%s ","V ");
            }
            else{
                printf("%s ","X ");
            }
        }
        printf("\n");
        i++;
    }
}

int eight_queen(int *board,int row){
    if (row == 8){
        print_board(board);
        return 1;
    }
    board[row] = 0;
    while (1){
        if (check(board,row) && eight_queen(board,row+1)){
             return 1;
        }
        else{
            if(++board[row] >= 8){
                break;
            }
        }
    }

    return 0;    
}

int main(){
    eight_queen(board,0);
    // print_board(board);
    return 0;
}