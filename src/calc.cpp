#include <stdio.h>
#include <stdlib.h>
#include <stack>
#include <iostream>
#include <string>
#include <cmath>

using namespace std;

/**************************************************************************
 * 功能：计算符号优先级
 * 入参：cSymbol 符号
 * 出参：无
 * 返回：优先级
 * 备注：数越小优先级越低
**************************************************************************/
int SymbolPriority ( char cSymbol )
{
    switch ( cSymbol )
    {
    case '+':
    case '-':
        return 1;
    case '*':
    case '/':
        return 2;
    case '(':
    default:
        return 0;
    }
}

/**************************************************************************
 * 功能：表达式格式转换
 * 入参：strInfix 中缀方式
 * 出参：strSuffix 后缀方式
 * 返回：是否成功  [false]失败 [true]成功
 * 备注：无
**************************************************************************/
bool FormatConv ( const string& strInfix, string& strSuffix )
{
    stack< char > stackChar;  // 字符栈
    int           i      = 0;
    char          cChar  = ' ';
    int           iTtype = 0; /* 0:运算符 1:数字 */
    int           iFlag  = 0; /* 是否是负数 0:否 1:是 */

    while ( i < strInfix.length () )
    {
        cChar = strInfix.at ( i++ );

        switch ( cChar )
        {
        case ' ':
            break;
        case '(':
            stackChar.push ( cChar );
            break;
        case ')':
            if ( stackChar.empty () ) /* 格式有误 */
                return false;

            while ( stackChar.top () != '(' )
            {
                strSuffix += stackChar.top ();
                stackChar.pop ();

                if ( stackChar.empty () ) /* 格式有误 */
                    return false;
            }
            stackChar.pop (); /*删除栈顶的(*/
            break;
        case '-': {
            if ( iTtype == 0 )
            {
                iFlag = 1;
                continue;
            }
        }
        case '+':
        case '*':
        case '/':
            while ( !stackChar.empty () && ( SymbolPriority ( stackChar.top () ) >= SymbolPriority ( cChar ) ) )
            {
                strSuffix += stackChar.top ();
                stackChar.pop ();
            }
            stackChar.push ( cChar );

            iTtype = 0;
            break;
        default:
            while ( ( cChar >= 48 && cChar <= 57 ) || cChar == 46 )
            {
                strSuffix += cChar;
                cChar = strInfix.at ( i++ );
            }
            i--;
            strSuffix += ' '; /* 分割数字 */

            if ( iFlag == 1 )
            {
                strSuffix += '~'; /* 负数标志 */
                strSuffix += ' '; /* 分割数字 */
                iFlag = 0;
            }

            iTtype = 1;
            break;
        } /* switch end */
    }     /* while end */

    while ( !stackChar.empty () )
    {
        strSuffix += stackChar.top ();
        stackChar.pop ();
    }

    strSuffix += ' '; /* 避免越界访问，致使程序异常退出 */

    return true;
}

/**************************************************************************
 * 功能：（后缀）表达式计算
 * 入参：strSuffix 后缀方式
 * 出参：pbOk 计算结果  [false]失败 [true]成功
 * 返回：计算值
 * 备注：无
**************************************************************************/
float CalcSuffixExpress ( const string& strSuffix, bool* pbOk )
{
    stack< float > stackDigit;
    int            i      = 0;
    float          fDig   = 0.0f;
    float          fSmall = 0.0f;
    float          fBase  = 0.0f;
    char           cChar  = ' ';

    if ( NULL == pbOk )
        return 0.0f;

    *pbOk = false;

    while ( i < strSuffix.length () )
    {
        cChar = strSuffix.at ( i++ );

        if ( cChar == ' ' )
            continue;

        fDig = 0;
        switch ( cChar )
        {
        case '+':
        case '-':
        case '*':
        case '/':
            if ( stackDigit.empty () ) /* 格式有误 */
                return 0.0f;
            fDig = stackDigit.top ();
            stackDigit.pop ();
            if ( stackDigit.empty () ) /* 格式有误 */
                return 0.0f;

            if ( '+' == cChar )
                fDig = stackDigit.top () + fDig;
            else if ( '-' == cChar )
                fDig = stackDigit.top () - fDig;
            else if ( '*' == cChar )
                fDig = stackDigit.top () * fDig;
            else
            {
                if ( fabs ( fDig ) < 0.000001 )
                    return 0.0f;
                fDig = stackDigit.top () / fDig;
            }

            stackDigit.pop ();
            break;
        case '~':
            if ( stackDigit.empty () ) /* 格式有误 */
                return 0.0f;
            fDig = stackDigit.top ();
            stackDigit.pop ();
            fDig = fDig * -1;
            break;
        default:
            while ( cChar >= 48 && cChar <= 57 )
            {
                fDig  = fDig * 10 + cChar - 48;
                cChar = strSuffix.at ( i++ );
            }
            if ( cChar == '.' )
            {
                fSmall = 0.0f;
                fBase  = 10.0f;
                cChar  = strSuffix.at ( i++ );
                while ( cChar >= 48 && cChar <= 57 )
                {
                    fSmall = fSmall + ( cChar - 48 ) / fBase;
                    fBase *= 10;
                    cChar = strSuffix.at ( i++ );
                }
                fDig += fSmall;
            }
            i--;
            break;
        } /* switch end */

        stackDigit.push ( fDig );
    } /* while end */

    if ( stackDigit.empty () )
        return 0.0f;

    fDig = stackDigit.top ();
    stackDigit.pop ();

    if ( !stackDigit.empty () )
        return 0.0f;

    *pbOk = true;

    return fDig;
}

/**************************************************************************
 * 功能：表达式检查
 * 入参：strExpress 表达式
 * 出参：无
 * 返回：是否通过  [false]未通过  [true]通过
 * 备注：无
**************************************************************************/
bool ExpressCheck ( const string& strExpress )
{
    int  i     = 0;
    int  iLen  = strExpress.length ();
    char cChar = ' ';
    if ( iLen <= 0 )
        return false;

    for ( i = 0; i < iLen; i++ )
    {
        cChar = strExpress.at ( i );

        if ( cChar != '+' &&
             cChar != '-' &&
             cChar != '*' &&
             cChar != '/' &&
             cChar != '(' &&
             cChar != ')' &&
             cChar != '.' &&
             ( cChar < 48 || cChar > 57 ) )
            return false;
    }

    return true;
}

/**************************************************************************
 * 功能：计算入口
 * 入参：pInfix 中缀表达式
 * 出参：pbOk 计算结果  [false]失败 [true]成功
 * 返回：计算值
 * 备注：无
**************************************************************************/
float CalcEntry ( char* pInfix, bool* pbOk )
{
    string strInfix  = "";  // 中缀方式
    string strSuffix = "";  // 后缀方式

    if ( NULL == pbOk )
        return 0.0f;

    // 计算结果默认失败
    *pbOk = false;

    if ( pInfix == NULL )
        return 0.0;

    strInfix = string ( pInfix );

    if ( !ExpressCheck ( strInfix ) )
        return 0.0;

    strInfix += ' '; /* 避免越界访问，致使程序异常退出 */

    // 中缀 转 后缀
    if ( !FormatConv ( strInfix, strSuffix ) )
        return 0.0f;

    return CalcSuffixExpress ( strSuffix, pbOk );
}

int main ( int argc, char* argv[] )
{
    bool  bOk  = false;
    float fRet = 0.0f;

    if ( 2 != argc )
        return 0;

    fRet = CalcEntry ( argv[ 1 ], &bOk );
    if ( !bOk )
    {
        printf ( "格式有误\n" );
        return 0;
    }

    printf ( "%s 计算结果为: %f\n", argv[ 1 ], fRet );

    return 0;
}
