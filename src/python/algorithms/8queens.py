def find_Queen(row):

    if row>7:
        global count
        count+=1
        print_queen()
        return

    for column in range(8):
        if check(row,column):
            Queen[row][column]=1
            find_Queen(row+1)
            Queen[row][column]=0

def check(row,column):

    for k in range(8):
        if Queen[k][column]==1:
            return False

    for i,j in zip(range(row-1,-1,-1),range(column-1,-1,-1)):
        if Queen[i][j]==1:
            return False

    for i,j in zip(range(row-1,-1,-1),range(column+1,8)):
        if Queen[i][j]==1:
            return False

    return True

def print_queen():
    print(Queen)
    for i in range(8):
        for j in range(8):
            if Queen[i][j]==1:
                print('X '*j+'Y '+'X '*(7-j))
    print("\n\n")

def main():
	global Queen
	global count
	find_Queen(8)
	print_queen()

if __name__ == '__main__':
    main()
