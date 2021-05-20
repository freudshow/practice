int exgcd(int a, int b, int& x, int& y) {
    if(a < b) return exgcd(b, a, y, x);
    int m = 0, n = 1;
    x = 1; y = 0;
    while(b != 0) {
        int d = a / b, t;
        t = m; m = x - d * t; x = t;
        t = n; n = y - d * t; y = t;
        t = a % b; a = b; b = t;
    }
    return a;
}