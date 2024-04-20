int a[5];
int a[6];

int min(int a, int b)
{
    if (a > b)
    {
        return a;
    }
    return b;
}

int main()
{
    return min(1, 2);
}