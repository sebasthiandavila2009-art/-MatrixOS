/*
 * MatrixOS MatrixFS
 * Version 0.2 - In-Memory Filesystem
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

static void fs_copy(char *destination, const char *source)
{
    int i = 0;

    while (source[i] && i < MATRIXFS_NAME_MAX - 1)
    {
        destination[i] = source[i];
        i++;
    }

    destination[i] = 0;
}

static void fs_data_copy(char *destination, const char *source)
{
    int i = 0;

    while (source[i] && i < MATRIXFS_DATA_MAX - 1)
    {
        destination[i] = source[i];
        i++;
    }

    destination[i] = 0;
}

static int fs_equal(const char *a, const char *b)
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

    fs_copy(matrixfs_entries[3].name, "README.TXT");
    fs_copy(matrixfs_entries[3].parent, "DOCUMENTS");
    fs_data_copy(matrixfs_entries[3].data, "WELCOME TO MATRIXOS");

    matrixfs_entries[3].size = 19;

    /* MATRIXOS.TXT */

    matrixfs_entries[4].used = 1;
    matrixfs_entries[4].directory = 0;

    fs_copy(matrixfs_entries[4].name, "MATRIXOS.TXT");
    fs_copy(matrixfs_entries[4].parent, "DOCUMENTS");
    fs_data_copy(matrixfs_entries[4].data, "MATRIXOS SYSTEM FILE");

    matrixfs_entries[4].size = 21;

    matrixfs_initialized = 1;
}


/* ========================================
   Find entry
   ======================================== */

int matrixfs_find(const char *parent, const char *name)
{
    int i;

    matrixfs_init();

    for (i = 0; i < MATRIXFS_MAX_FILES; i++)
    {
        if (!matrixfs_entries[i].used)
            continue;

        if (!fs_equal(matrixfs_entries[i].parent, parent))
            continue;

        if (!fs_equal(matrixfs_entries[i].name, name))
            continue;

        return i;
    }

    return -1;
}


/* ========================================
   Find free slot
   ======================================== */

static int matrixfs_find_free(void)
{
    int i;

    for (i = 0; i < MATRIXFS_MAX_FILES; i++)
    {
        if (!matrixfs_entries[i].used)
            return i;
    }

    return -1;
}


/* ========================================
   Create file
   ======================================== */

int matrixfs_create_file(
    const char *parent,
    const char *name,
    const char *data)
{
    int index;

    matrixfs_init();

    if (!name || !name[0])
        return -1;

    if (matrixfs_find(parent, name) >= 0)
        return -1;

    index = matrixfs_find_free();

    if (index < 0)
        return -1;

    matrixfs_entries[index].used = 1;
    matrixfs_entries[index].directory = 0;

    fs_copy(matrixfs_entries[index].name, name);
    fs_copy(matrixfs_entries[index].parent, parent);

    if (data)
        fs_data_copy(matrixfs_entries[index].data, data);
    else
        matrixfs_entries[index].data[0] = 0;

    index = matrixfs_find(parent, name);

    if (index >= 0)
    {
        int size = 0;

        while (matrixfs_entries[index].data[size] &&
               size < MATRIXFS_DATA_MAX - 1)
        {
            size++;
        }

        matrixfs_entries[index].size = size;
    }

    return index;
}


/* ========================================
   Create directory
   ======================================== */

int matrixfs_create_directory(
    const char *parent,
    const char *name)
{
    int index;

    matrixfs_init();

    if (!name || !name[0])
        return -1;

    if (matrixfs_find(parent, name) >= 0)
        return -1;

    index = matrixfs_find_free();

    if (index < 0)
        return -1;

    matrixfs_entries[index].used = 1;
    matrixfs_entries[index].directory = 1;
    matrixfs_entries[index].size = 0;

    fs_copy(matrixfs_entries[index].name, name);
    fs_copy(matrixfs_entries[index].parent, parent);

    matrixfs_entries[index].data[0] = 0;

    return index;
}


/* ========================================
   Delete entry
   ======================================== */

int matrixfs_delete(int index)
{
    matrixfs_init();

    if (index < 0 ||
        index >= MATRIXFS_MAX_FILES ||
        !matrixfs_entries[index].used)
    {
        return 0;
    }

    /*
     * Do not allow deleting built-in root folders
     * for now.
     */
    if (index <= 2)
        return 0;

    matrixfs_entries[index].used = 0;
    matrixfs_entries[index].directory = 0;
    matrixfs_entries[index].size = 0;

    matrixfs_entries[index].name[0] = 0;
    matrixfs_entries[index].parent[0] = 0;
    matrixfs_entries[index].data[0] = 0;

    return 1;
}


/* ========================================
   Rename entry
   ======================================== */

int matrixfs_rename(
    int index,
    const char *new_name)
{
    matrixfs_init();

    if (index < 0 ||
        index >= MATRIXFS_MAX_FILES ||
        !matrixfs_entries[index].used)
    {
        return 0;
    }

    if (!new_name || !new_name[0])
        return 0;

    if (index <= 2)
        return 0;

    if (matrixfs_find(
            matrixfs_entries[index].parent,
            new_name) >= 0)
    {
        return 0;
    }

    fs_copy(
        matrixfs_entries[index].name,
        new_name
    );

    return 1;
}


/* ========================================
   Write file
   ======================================== */

int matrixfs_write(
    int index,
    const char *data)
{
    int size = 0;

    matrixfs_init();

    if (index < 0 ||
        index >= MATRIXFS_MAX_FILES ||
        !matrixfs_entries[index].used ||
        matrixfs_entries[index].directory)
    {
        return 0;
    }

    if (!data)
        data = "";

    fs_data_copy(
        matrixfs_entries[index].data,
        data
    );

    while (matrixfs_entries[index].data[size] &&
           size < MATRIXFS_DATA_MAX - 1)
    {
        size++;
    }

    matrixfs_entries[index].size = size;

    return 1;
}


/* ========================================
   Check directory
   ======================================== */

int matrixfs_is_directory(int index)
{
    if (index < 0 ||
        index >= MATRIXFS_MAX_FILES ||
        !matrixfs_entries[index].used)
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
    if (index < 0 ||
        index >= MATRIXFS_MAX_FILES ||
        !matrixfs_entries[index].used)
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
    if (index < 0 ||
        index >= MATRIXFS_MAX_FILES ||
        !matrixfs_entries[index].used ||
        matrixfs_entries[index].directory)
    {
        return "";
    }

    return matrixfs_entries[index].data;
}


/* ========================================
   Get file size
   ======================================== */

int matrixfs_size(int index)
{
    if (index < 0 ||
        index >= MATRIXFS_MAX_FILES ||
        !matrixfs_entries[index].used)
    {
        return 0;
    }

    return matrixfs_entries[index].size;
}


/* ========================================
   List directory
   ======================================== */

int matrixfs_list(
    const char *parent,
    int *results,
    int max_results)
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
        {
            continue;
        }

        if (count >= max_results)
            break;

        results[count] = i;
        count++;
    }

    return count;
}
