#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "utils.h"
#include "system.h"

#ifdef ALT_STDOUT_IS_JTAG_UART
#include "HAL/inc/sys/ioctl.h"
#include "altera_avalon_jtag_uart.h"
// Due to the amount of potential status information make sure we have a big enough UART FIFO
#if ALTERA_AVALON_JTAG_UART_BUF_LEN < 8192
#error "Please increase ALTERA_AVALON_JTAG_UART_BUF_LEN to at least 8192"
#endif // ALTERA_AVALON_JTAG_UART_BUF_LEN
#endif // ALT_STDOUT_IS_JTAG_UART

// The current logging level for the application
static eLogLevel _g_log_level = kInfo;

// int to indicate that an active juart is connected, 1 = connected
static int _g_juart_connected = 1;

/**
 * @brief Sets the logging level for the application.
 *
 * This function sets the logging level to the specified new level and returns the old level.
 *
 * @param[in] new_level The new logging level to set.
 * @return The old logging level.
 */
eLogLevel set_log_level(const eLogLevel new_level)
{
    eLogLevel old_level = _g_log_level;
    _g_log_level = new_level;
    return old_level;
}

/**
 * @brief Elevates the logging level to at least the specified new level.
 *
 * This function increases the logging level if the new level is higher than the current level.
 *
 * @param[in] new_level The new logging level to elevate to.
 * @return The old logging level.
 */
eLogLevel elevate_log_level(const eLogLevel new_level)
{
    eLogLevel old_level = _g_log_level;
    if (new_level > old_level)
    {
        _g_log_level = new_level;
    }
    return old_level;
}

/**
 * @brief Gets the current logging level.
 *
 * This function returns the current logging level of the application.
 *
 * @return The current logging level.
 */
eLogLevel get_log_level()
{
    return _g_log_level;
}

//==================================================================

/**
 * @brief Converts a logging level to its string representation.
 *
 * This function returns a string representation of the given logging level.
 *
 * @param[in] level The logging level to convert.
 * @return A string representation of the logging level.
 */
const char *logLevelToString(eLogLevel level)
{
    switch (level)
    {
    case kError:
        return "Error  ";
    case kWarning:
        return "Warn   ";
    case kInfo:
        return "Info   ";
    case kDebug:
        return "Debug  ";
    case kVerbose:
        return "Verbose";
    default:
        return "Unknown";
    }
}

//==================================================================

/**
 * @brief Logs a message with the specified logging level and identifier.
 *
 * This function prints a formatted log message if the logging level is within the current level.
 *
 * @param[in] level The logging level of the message.
 * @param[in] pId The identifier for the log message.
 * @param[in] format The format string for the log message.
 * @param[in] ... Additional arguments for the format string.
 */
void logMessagePrint(eLogLevel level, const char *pId, const char *format, ...)
{
    // Due to a bug with formatted string when the JUART FIFO fills up we want to make sure
    // that we only print characters when a juart host is connected
    if( _g_juart_connected )
    {
        if ((level & ~kSuppress) > _g_log_level)
        {
            return;
        }
        // Do we want to print the level information?
        if (!(level & kSuppress))
        {
            if (pId)
            {
                printf("%3s %s : ", pId, logLevelToString(level));
            }
            else
            {
                printf("    %s : ", logLevelToString(level));
            }
        }
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}


//==================================================================
// Menu Subsystem

#define logPrint(level, format, ...) LOG_EXPAND(logMessagePrint(level, "Utl", format, ##__VA_ARGS__))
#define INDENT_WIDTH 5


/**
 * @brief Executes a menu item based on the provided code.
 *
 * This function executes the action associated with the menu item matching the provided code.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the menu item to execute.
 * @param[in] pMenuList Pointer to the start of a menu list
 * @return True if the menu item is successfully executed, false otherwise.
 */
bool executeMenuItem(utils_context_t *pContext, const char *pCode, const struct MenuItem *pMenuList)
{
    bool bExecuted = false;
    if ((pContext == NULL) || (pCode == NULL))
    {
        return bExecuted;
    }
    const struct MenuItem *pItem = pMenuList;
    while (pItem->pDescription)
    {
        if (pItem->action) // If we have an action then check for the code
        {
            if (strcmp(pCode, pItem->code) == 0)
            {
                // Elevate the log level so we guarantee to see it
                eLogLevel current_level = elevate_log_level(kInfo);
                logPrint(kInfo | kSuppress, "\n");
                logPrint(kInfo, "< %s : %s\n", pItem->pDescription, pCode);
                (pItem->action)(pContext, pCode, pItem->pExtItems?pItem->pExtItems:pMenuList,pItem->metadata);

                eLogLevel level = get_log_level();
                // if the action has changed the level then return to that level
                if (level != kInfo)
                {
                    current_level = elevate_log_level(kInfo);
                }
                logPrint(kInfo, ">\n"); // Automation termination
                set_log_level(current_level);
                bExecuted = true;
                break;
            }
        }

        // If the item has not been executed then check to see if we have a secondary list attached to this item
        if (!bExecuted && (pItem->pExtItems))
        {
            bExecuted = executeMenuItem(pContext, pCode, pItem->pExtItems);
            if (bExecuted)
            {
                break; // The command was executed
            }
        }
        pItem++;
    }
    return bExecuted;
}

//==================================================================

/**
 * @brief Reads a key from the standard input.
 *
 * This function reads a single character from the standard input and returns it.
 *
 * @return The character read from the standard input, or 0 if no character is read.
 */
char get_key()
{
    char key;
    int d, i;
    char *c = (char *)&d;

    i = read(0, (void *)&d, 1); // 0 = stdin
    if ((i < 1) || (*c == EOF))
        return 0;

    key = *c;
    return key;
}

/**
 * @brief Displays help information for the menu.
 *
 * This function logs help information for the available menu items.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
bool DisplayHelp(utils_context_t *pContext, const char *pCode, const struct MenuItem *pMenuList)
{
    UNUSED(pCode);
    UNUSED(pContext);
    logPrint(kInfo, "************ Help ************\n");
    if (pMenuList != NULL)
    {
        const struct MenuItem *pItem = pMenuList;
        while (pItem->pDescription)
        {
            if (pItem->action != NULL)
            {
                logPrint(kInfo, "%*s : %s\n", (2 * INDENT_WIDTH), pItem->code, pItem->pDescription);
            }
            else
            {
                logPrint(kInfo, "     %s\n", pItem->pDescription);
            }
            pItem++;
        }
    }
    logPrint(kInfo, "******************************\n");
    return true;
}

/**
 * @brief Checks and processes menu input.
 *
 * This function checks for user input and processes menu commands based on the input.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pMenuItems Pointer to an array of MenuItems
 */
void CheckMenuInput(utils_context_t *pContext, const struct MenuItem *pMenuItems)
{
    #ifdef ALT_STDOUT_IS_JTAG_UART
    // Due to a bug with formatted string when the JUART FIFO fills up we want to make sure
    // that we only print characters when a juart host is connected
    {
        static int check_juart_connected = 0;
        check_juart_connected++;
        if( check_juart_connected > 10 )
        {
            check_juart_connected = 0;
            int value;
            ioctl(STDOUT_FILENO, TIOCGCONNECTED, &value);
            if( value != _g_juart_connected )
            {
                _g_juart_connected = value;
                if( _g_juart_connected == 1 )
                {
                    printf("\nJuart Terminal Connected\n");
                }
                else
                {
                    printf("\nJuart Terminal Disconnected\n");
                }
            }
        }
    }
    #endif

    unsigned char key = get_key();
    if (key)
    {
        if ( (key == '\n') || (key == '\r'))
        {
            // Elevate the log level so we guarantee to see it
            eLogLevel current_level = elevate_log_level(kInfo);
            // Display help if aborting the command
            logPrint(kInfo, "< %s : %s\n", "Invalid Command", pContext->menu.input);
            DisplayHelp(pContext, "", pMenuItems);
            logPrint(kInfo, ">\n"); // Automation termination
            set_log_level(current_level);
            pContext->menu.input_idx = 0;
            pContext->menu.input[pContext->menu.input_idx] = '\0';
        }
        else if ((key == 127) || (key == 8)) // Handle backspace (127 is DEL, 8 is BS)
        {
            if (pContext->menu.input_idx > 0)
            {
                pContext->menu.input_idx--;
                pContext->menu.input[pContext->menu.input_idx] = '\0';
                logPrint(kInfo | kSuppress, "\b \b");
            }
        }
        else if (isalnum(key) && (pContext->menu.input_idx < MAX_MENU_CODE))
        {
            logPrint(kInfo | kSuppress, "%c", key);
            pContext->menu.input[pContext->menu.input_idx++] = key;
            pContext->menu.input[pContext->menu.input_idx] = '\0';
            if (executeMenuItem(pContext, pContext->menu.input, pMenuItems))
            {
                pContext->menu.input_idx = 0;
            }
            else if (pContext->menu.input_idx == MAX_MENU_CODE)
            {
                // We attempted execution but not valid so flush command
                pContext->menu.input_idx = 0;
                logPrint(kInfo | kSuppress, "\n");
                DisplayHelp(pContext, "", pMenuItems);
            }
        }
        else
        {
            // Discard
            pContext->menu.input_idx = 0;
        }
    }
}

//==================================================================

/**
 * @brief Executes utility menu functions based on the provided code.
 *
 * This function performs actions such as setting log levels and clearing the screen based on the provided code.
 *
 * @param[in] pContext Pointer to the application context.
 * @param[in] pCode The code representing the action to perform.
 * @param[in] pMenuList Poiner to an array of MenuItems.
 * @return True if the help information is successfully displayed, false otherwise.
 */
bool UtilsMenuFunction(utils_context_t *pUtlContext, const char *pCode, const struct MenuItem *pMenuList, uint32_t metadata)
{
    if ((pUtlContext == NULL) || (pCode == NULL))
    {
        return false;
    }
    if (strcmp(pCode, "le") == 0 )
    {
        set_log_level(kError);
    }
    else if (strcmp(pCode, "lw") == 0)
    {
        set_log_level(kWarning);
    }
    else if (strcmp(pCode, "li") == 0)
    {
        set_log_level(kInfo);
    }
    else if (strcmp(pCode, "ld") == 0)
    {
        set_log_level(kDebug);
    }
    else if (strcmp(pCode, "lv") == 0)
    {
        set_log_level(kVerbose);
    }
    else if (strcmp(pCode, "lm") == 0)
    {
        set_log_level(kMaxLogLevel);
    }
    else if (strcmp(pCode, "ls") == 0)
    {
        set_log_level(kSilent);
    }
    else if (strcmp(pCode, "c") == 0)
    {
        // Clear the screen and move the cursor to the home position
        printf("\033[2J\033[H");
    }
    return true;
}

/**
 * @brief List of Logging Menu Items
 */
struct MenuItem _LogMenuItems[] = 
{
    {"", "Logging", NULL},
    {"ls", "Silent", UtilsMenuFunction, NULL},
    {"le", "Error", UtilsMenuFunction, NULL},
    {"lw", "Warning", UtilsMenuFunction, NULL},
    {"li", "Info", UtilsMenuFunction, NULL},
    {"ld", "Debug", UtilsMenuFunction, NULL},
    {"lv", "Verbose", UtilsMenuFunction, NULL},
    {"lm", "Max", UtilsMenuFunction, NULL},
    {"c", "Clear", UtilsMenuFunction, NULL},
    {"", NULL, NULL} // Terminate the list
};
