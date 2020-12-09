#include <iostream>
 
using namespace std;
 
int Horner_rule(int arr[],int n,int x)
{
    int i,ans = 0;;
    for(i = 0;i<n;i++)
    {
        ans =arr[i]+x*ans;
    }
    return ans;
}
int main()
{
    int n,x,i,t,arr[1000];
    cin>>t;
    while(t--)
    {
        cout<<"输入 : n , x : ";
        cin>>n>>x;
        cout<<"输入 "<<n <<" 个数  : ";
        for(i = n-1;i>=0;i--)//求值是从 an 开始，所以倒着存
            cin>>arr[i];
        for(i = 0;i<n-1;i++)
            cout<<arr[i]<<"*x"<<"^"<<i<<"+";
        cout<<arr[i]<<"*x"<<"^"<<i<<" = ";
        cout<<Horner_rule(arr,n,x)<<endl;
    }
    return 0;
}