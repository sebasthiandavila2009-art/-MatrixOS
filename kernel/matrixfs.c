/*
 * MatrixOS MatrixFS
 * Version 0.1 - In-Memory Filesystem
 */

#define MATRIXFS_MAX_FILES 16
#define MATRIXFS_NAME_MAX 32
#define MATRIXFS_DATA_MAX 256

typedef struct
{
    int used;
    int directory;

    char name[MATRIXFS_NAME_MAX];
    char parent[MATRIXFS_NAME_MAX];

    char data[MATRIXFS_DATA_MAX];
    int size;

} matrixfs_entry_t;


static matrixfs_entry_t matrixfs_entries[MATRIXFS_MAX_FILES];

static int matrixfs_initialized = 0;


/* ========================================
   Internal string helpers
   ======================================== */

static void fs_copy(
    char *destination,
    const char *source
)
{
    int i = 0;

    while (
        source[i] &&
        i < MATRIXFS_NAME_MAX - 1
    )
    {
        destination[i] = source[i];
        i++;
    }

    destination[i] = 0;
}


static int fs_equal(
    const char *a,
    const char *b
)
{
    while (*a && *b)
    {
        if (*a != *b)
            return 0;

        a++;
        b++;
    }

    return *a == 0 && *b == 0;
}


/* ========================================
   Filesystem initialization
   ======================================== */

void matrixfs_init(void)
{
    int i;

    if (matrixfs_initialized)
        return;

    for (i = 0; i < MATRIXFS_MAX_FILES; i++)
    {
        matrixfs_entries[i].used = 0;
        matrixfs_entries[i].directory = 0;
        matrixfs_entries[i].size = 0;

        matrixfs_entries[i].name[0] = 0;
        matrixfs_entries[i].parent[0] = 0;
        matrixfs_entries[i].data[0] = 0;
    }

    /* Root directories. */

    matrixfs_entries[0].used = 1;
    matrixfs_entries[0].directory = 1;
    fs_copy(matrixfs_entries[0].name, "SYSTEM");
    fs_copy(matrixfs_entries[0].parent, "/");

    matrixfs_entries[1].used = 1;
    matrixfs_entries[1].directory = 1;
    fs_copy(matrixfs_entries[1].name, "DOCUMENTS");
    fs_copy(matrixfs_entries[1].parent, "/");

    matrixfs_entries[2].used = 1;
    matrixfs_entries[2].directory = 1;
    fs_copy(matrixfs_entries[2].name, "DOWNLOADS");
    fs_copy(matrixfs_entries[2].parent, "/");

    /* README.TXT */

    matrixfs_entries[3].used = 1;
    matrixfs_entries[3].directory = 0;

    fs_copy(
        matrixfs_entries[3].name,
        "README.TXT"
    );

    fs_copy(
        matrixfs_entries[3].parent,
        "DOCUMENTS"
    );

    fs_copy(
        matrixfs_entries[3].data,
        "WELCOME TO MATRIXOS"
    );

    matrixfs_entries[3].size = 19;

    /* MATRIXOS.TXT */

    matrixfs_entries[4].used = 1;
    matrixfs_entries[4].directory = 0;

    fs_copy(
        matrixfs_entries[4].name,
        "MATRIXOS.TXT"
    );

    fs_copy(
        matrixfs_entries[4].parent,
        "DOCUMENTS"
    );

    fs_copy(
        matrixfs_entries[4].data,
        "MATRIXOS SYSTEM FILE"
    );

    matrixfs_entries[4].size = 21;

    matrixfs_initialized = 1;
}


/* ========================================
   Find entry
   ======================================== */

int matrixfs_find(
    const char *parent,
    const char *name
)
{
    int i;

    matrixfs_init();

    for (i = 0; i < MATRIXFS_MAX_FILES; i++)
    {
        if (!matrixfs_entries[i].used)
            continue;

        if (!fs_equal(
                matrixfs_entries[i].parent,
                parent))
            continue;

        if (!fs_equal(
                matrixfs_entries[i].name,
                name))
            continue;

        return i;
    }

    return -1;
}


/* ========================================
   Check directory
   ======================================== */

int matrixfs_is_directory(int index)
{
    if (
        index < 0 ||
        index >= MATRIXFS_MAX_FILES ||
        !matrixfs_entries[index].used
    )
    {
        return 0;
    }

    return matrixfs_entries[index].directory;
}


/* ========================================
   Get entry name
   ======================================== */

const char *matrixfs_name(int index)
{
    if (
        index < 0 ||
        index >= MATRIXFS_MAX_FILES ||
        !matrixfs_entries[index].used
    )
    {
        return "";
    }

    return matrixfs_entries[index].name;
}


/* ========================================
   Read file
   ======================================== */

const char *matrixfs_read(int index)
{
    if (
        index < 0 ||
        index >= MATRIXFS_MAX_FILES ||
        !matrixfs_entries[index].used ||
        matrixfs_entries[index].directory
    )
    {
        return "";
    }

    return matrixfs_entries[index].data;
}


/* ========================================
   List directory
   ======================================== */

int matrixfs_list(
    const char *parent,
    int *results,
    int max_results
)
{
    int count = 0;
    int i;

    matrixfs_init();

    for (i = 0; i < MATRIXFS_MAX_FILES; i++)
    {
        if (!matrixfs_entries[i].used)
            continue;

        if (!fs_equal(
                matrixfs_entries[i].parent,
                parent))
            continue;

        if (count >= max_results)
            break;

        results[count] = i;
        count++;
    }

    return count;
}
