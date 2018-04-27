%{
#include <stdio.h>
#include <ctype.h>
#include <math.h>
%}

%union {
    double dval;
}

%token <dval> NUMBER
%token POW

%left '+' '-'
%left '*' '/'
%nonassoc UMINUS  
%right POW

%type <dval> expr

%%

line
    : expr '\n'             { printf("%lf\n", $1); }
    ;

expr
    : NUMBER                { $$ = $1; }
    | expr '+' expr         { $$ = $1 + $3; }
    | expr '-' expr         { $$ = $1 - $3; }
    | expr '*' expr         { $$ = $1 * $3; }
    | expr '/' expr         {
        if (0 == $3) yyerror("divided by zero");
        else $$ = $1 / $3;
    }
    | '-' expr %prec UMINUS { $$ = -$2; }
    | expr POW expr         { $$ = pow($1, $3); }
    | '(' expr ')'          { $$ = $2; }
    ;

%%

int main() {
    return yyparse();
}

int yyerror(char* s) {
    fprintf(stderr, "%s\n", s);
    return 1;
}

int end = 0;

int yylex() {
    if (end) return 0;
    int c;

    /* skip space */
    while (' ' == (c = getchar())) { }

    if (isdigit(c)) {
        ungetc(c, stdin);
        scanf("%lf", &yylval);
        return NUMBER;
    }

    if ('*' == c) {
        c = getchar();
        if ('*' == c) {
            return POW;
        } else {
            ungetc(c, stdin);
            return '*';
        }
    }

    if ('\n' == c) {
        end = 1;
    }

    return c;
}
