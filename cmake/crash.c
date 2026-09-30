/* Crashes on purpose, so the naive runner's crash handling has a test. */
int main(void)
{
    volatile int *null = 0;
    *null = 1;
    return 0;
}
