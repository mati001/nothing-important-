#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
// * Signal registration (`sigaction`)
// * Signal blocking/unblocking (`sigprocmask`)
// * Handling pending signals (`sigpending`, `sigismember`)

extern sigset_t blockSet;

void handle_signal_FM(int sig)
{
    switch (sig)
    {
    case SIGINT:
        printf("[Outcome:] The TA announced: Everyone get 100 on the exercise!\n");
        break;
    case SIGFPE:
        printf("[Outcome:] You picked it up just in time.\n");
        break;
    case SIGBUS:
        printf("[Outcome:] Food delivery is here.\n");
        break;
    default:
        break;
    }
}
void use_input(char input)
{
    switch (input)
    {
    case '1':
        raise(SIGINT); // Simulate email notification
        break;
    case '2':
        raise(SIGFPE); // Simulate reminder to pick up delivery
        break;
    case '3':
        raise(SIGBUS); // Simulate doorbell ringing
        break;
    case 'q':
        break; // Quit the round
    }
}
void start_round_print(int round)
{
    printf("══════════════════════════════════════════════\n");
    printf("                Focus Round %d                \n", round);
    printf("──────────────────────────────────────────────\n");
}
void before_distraction_print()
{
    printf("Simulate a distraction:\n");
    printf("  1 = Email notification\n");
    printf("  2 = Reminder to pick up delivery\n");
    printf("  3 = Doorbell Ringing\n");
    printf("  q = Quit\n");
}
void finish_round_print()
{
    printf("\n");
    printf("══════════════════════════════════════════════\n");
    printf("             Back to Focus Mode.              \n");
    printf("──────────────────────────────────────────────\n");
}
void checking_pending_print()
{
    printf(">> ──────────────────────────────────────────────\n");
    printf("        Checking pending distractions...      \n");
    printf("──────────────────────────────────────────────\n");
}
void register_handlers()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal_FM;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
}
void check_pending_signals()
{
    checking_pending_print();
    sigset_t pendingSet;
    sigemptyset(&pendingSet);
    sigpending(&pendingSet);
    sigset_t unblockSet;

    // Unblock SIGINT
    sigemptyset(&unblockSet);
    bool isPending = false;
    if (sigismember(&pendingSet, SIGINT))
    {
        isPending = true;
        printf(" - Email notification is waiting.\n");
        sigaddset(&unblockSet, SIGINT);
        sigprocmask(SIG_UNBLOCK, &unblockSet, NULL);
    }
    if (sigismember(&pendingSet, SIGFPE))
    {
        isPending = true;
        printf(" - You have a reminder to pick up your delivery.\n");
        sigaddset(&unblockSet, SIGFPE);
        sigprocmask(SIG_UNBLOCK, &unblockSet, NULL);
    }
    if (sigismember(&pendingSet, SIGBUS))
    {
        isPending = true;
        printf(" - The doorbell is ringing.\n");
        sigaddset(&unblockSet, (SIGBUS));
        sigprocmask(SIG_UNBLOCK, &unblockSet, NULL);
    }
    if (!isPending)
    {
        printf("No distractions reached you this round.\n");
    }
}

void run_round(int duartion, int round)
{
    start_round_print(round+1);
    sigprocmask(SIG_BLOCK, &blockSet, NULL); // Block all signals
    register_handlers();                     // Register signal handlers
    char input='\0';
    bool skipped = false;
    while (input != 'q' && duartion > 0) // Simulate work by busy waiting
    {
        before_distraction_print();
        input = getchar();
        while (getchar() != '\n'); // Clear the input buffer
        // need to flash the buffer
        use_input(input);
        duartion--;
    }
    check_pending_signals();
    finish_round_print();
}

void runFocusMode(int numOfRounds, int duartion)
{
    sigemptyset(&blockSet);
    sigaddset(&blockSet, SIGINT); // Block SIGINT
    sigaddset(&blockSet, SIGFPE); // Block SIGFPE
    sigaddset(&blockSet, SIGBUS); // Block SIGBUS
    printf("Entering Focus Mode. All distractions are blocked.\n");
    for (int i = 0; i < numOfRounds; i++)
    {
        run_round(duartion, i);
    }

    printf("Focus Mode complete. All distractions are now unblocked.\n");
}