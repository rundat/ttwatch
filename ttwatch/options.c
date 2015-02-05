/******************************************************************************\
** options.c                                                                  **
** Implementation of basic config file loading                                **
\******************************************************************************/

#include "options.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*****************************************************************************/
int split_conf_line(char *line, char **option, char **value)
{
    char *first, *last;
    last = strchr(line, '=');
    if (!last)
        return 0;

    *option = *value = 0;

    /* extract the option name */
    first = line;
    while (isspace(*first))
        ++first;
    --last;
    while (isspace(*last))
        --last;
    if (last > first)
    {
        *option = malloc(last - first + 1 + 1);
        memcpy(*option, first, last - first + 1);
        (*option)[last - first + 1] = 0;
    }

    if (*option)
    {
        /* extract the option value */
        first = strchr(line, '=') + 1;
        while (*first && isspace(*first))
            ++first;
        last = first + strlen(first) - 1;
        while (isspace(*last))
            --last;
        if (last > first)
        {
            *value = malloc(last - first + 1 + 1);
            memcpy(*value, first, last - first + 1);
            (*value)[last - first + 1] = 0;
        }
    }

    return 1;
}

/*****************************************************************************/
int get_bool(char *value, int *result)
{
    if (!strcasecmp(value, "y") || !strcasecmp(value, "yes") || !strcasecmp(value, "true"))
    {
        *result = 1;
        return 1;
    }
    else if (!strcasecmp(value, "n") || !strcasecmp(value, "no") || !strcasecmp(value, "false"))
    {
        *result = 0;
        return 1;
    }
    return 0;
}

/*****************************************************************************/
void load_conf_file(const char *filename, OPTIONS *options, ConfLoadType load_type)
{
    char str[256];
    FILE *file = 0;
    int global = 0;

    /* open the conf file */
    file = fopen(filename, "r");
    if (!file)
        return;

    write_log(0, "Loading %s\n", filename);

    while (!feof(file))
    {
        char *option, *value;
        int result = 0;

        if (!fgets(str, sizeof(str), file))
            break;

        /* remove any comments in the line */
        option = strchr(str, '#');
        if (option)
            *option = 0;    /* truncate the line at the '#' */

        /* check for a blank line */
        option = str;
        while (*option && isspace(*option))
            ++option;
        if (!*option)
            continue;

        /* make sure the line is valid */
        if (!split_conf_line(str, &option, &value))
        {
            write_log(1, "Invalid conf file line: %s\n", str);
            continue;
        }
        if (!option)
            continue;

        /* look for valid options/values */
        if (!strcasecmp(option, "ActivityStore"))
        {
            if (load_type != LoadDaemonOperations)
            {
                options->activity_store = value;
                value = 0;
            }
            result = 1;
        }
        else if (!strcasecmp(option, "Formats"))
        {
            if (load_type != LoadDaemonOperations)
            {
                options->formats = value;
                value = 0;
            }
            result = 1;
        }
        else if (!strcasecmp(option, "PostProcessor"))
        {
            if (load_type != LoadDaemonOperations)
            {
                options->post_processor = value;
                value = 0;
            }
            result = 1;
        }
        else if (!strcasecmp(option, "RunAsUser"))
        {
            result = global;
            if (load_type != LoadDaemonOperations)
            {
                if (global)
                {
                    options->run_as = 1;
                    options->run_as_user = value;
                }
            }
        }
        else if (!strcasecmp(option, "Device"))
        {
            if (load_type != LoadDaemonOperations)
            {
                options->select_device = 1;
                options->device = value;
                value = 0;
            }
            result = 1;
        }
        else if (!strcasecmp(option, "UpdateFirmware"))
        {
            result = 1;
            if (load_type != LoadSettingsOnly)
                result = get_bool(value, &options->update_firmware);
        }
        else if (!strcasecmp(option, "UpdateGPS"))
        {
            result = 1;
            if (load_type != LoadSettingsOnly)
                result = get_bool(value, &options->update_gps);
        }
        else if (!strcasecmp(option, "SetTime"))
        {
            result = 1;
            if (load_type != LoadSettingsOnly)
                result = get_bool(value, &options->set_time);
        }
        else if (!strcasecmp(option, "GetActivities"))
        {
            result = 1;
            if (load_type != LoadSettingsOnly)
                result = get_bool(value, &options->get_activities);
        }
        if (!strcasecmp(option, "SkipElevation"))
            result = get_bool(value, &options->skip_elevation);

        if (!result)
            write_log(0, "Invalid conf file line: %s\n", str);

        if (option)
            free(option);
        if (value)
            free(value);

        if (!result)
            break;
    }

    fclose(file);
}
