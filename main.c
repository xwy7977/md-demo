#include "./md4c/src/md4c.h"

#include <stdio.h>
#include <stdlib.h>

char result[1024] = {0};
unsigned int result_len = 0;

void string_append(const char *str, unsigned int len)
{
    if (result_len + len < sizeof(result))
    {
        memcpy(result + result_len, str, len);
        result_len += len;
        result[result_len] = '\n'; // 换行符结尾
        result_len++;
    }
    else
    {
        fprintf(stderr, "Result buffer overflow\n");
    }
}

int enter_block_callback(MD_BLOCKTYPE type, void *detail, void *userdata)
{
    printf("Entering block of type: %d\n", type);

    static const MD_CHAR *head[6] = {"<h1>", "<h2>", "<h3>", "<h4>", "<h5>", "<h6>"};

    switch (type)
    {
    case MD_BLOCK_H:
        string_append(head[((MD_BLOCK_H_DETAIL *)detail)->level - 1], strlen(head[((MD_BLOCK_H_DETAIL *)detail)->level - 1]));
        break;
    }

    return 0;
}

int leave_block_callback(MD_BLOCKTYPE type, void *detail, void *userdata)
{
    printf("Leaving block of type: %d\n", type);

    static const MD_CHAR *head[6] = {"</h1>\n", "</h2>\n", "</h3>\n", "</h4>\n", "</h5>\n", "</h6>\n"};

    switch (type)
    {
    case MD_BLOCK_H:
        string_append(head[((MD_BLOCK_H_DETAIL *)detail)->level - 1], strlen(head[((MD_BLOCK_H_DETAIL *)detail)->level - 1]));
        break;
    }
    return 0;
}

int enter_span_callback(MD_SPANTYPE type, void *detail, void *userdata)
{
    printf("Entering span of type: %d\n", type);

    switch (type)
    {
    case MD_SPAN_STRONG:
        string_append("<strong>", strlen("<strong>"));
        break;
    }

    return 0;
}

int leave_span_callback(MD_SPANTYPE type, void *detail, void *userdata)
{
    printf("Leaving span of type: %d\n", type);

    switch (type)
    {
    case MD_SPAN_STRONG:
        string_append("</strong>", strlen("</strong>"));
        break;
    }

    return 0;
}

int text_callback(MD_TEXTTYPE type, const MD_CHAR *text, MD_SIZE size, void *userdata)
{
    printf("Text of type: %d.\n", type);
    printf("%s\n", text);
    string_append(text, strlen(text));
    return 0;
}

void debug_log_callback(const char *msg, void *userdata)
{
    fprintf(stderr, "Debug: %s\n", msg);
}

void syntax_callback(void)
{
    printf("Syntax callback called.\n");
}

// 读取文本文件内容为字符串，返回指针和长度，失败返回NULL
char *read_file_to_string(const char *filename, size_t *out_size)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
    {
        perror("Failed to open file");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    if (size < 0)
    {
        fclose(f);
        return NULL;
    }
    fseek(f, 0, SEEK_SET);

    char *buffer = (char *)malloc(size + 1);
    if (!buffer)
    {
        fclose(f);
        return NULL;
    }

    size_t read_size = fread(buffer, 1, size, f);
    fclose(f);

    if (read_size != (size_t)size)
    {
        free(buffer);
        return NULL;
    }

    buffer[size] = '\0';
    if (out_size)
        *out_size = size;
    return buffer;
}

int main()
{
    size_t md_size;
    MD_CHAR *md_text = read_file_to_string("./../example.md", &md_size);
    if (!md_text)
    {
        fprintf(stderr, "Failed to read Markdown file.\n");
        return 1;
    }

    MD_PARSER parser = {0};
    parser.abi_version = 0;
    parser.flags = 0; // default value, follow the CommonMark specification
    parser.enter_block = enter_block_callback;
    parser.leave_block = leave_block_callback;
    parser.enter_span = enter_span_callback;
    parser.leave_span = leave_span_callback;
    parser.text = text_callback;
    parser.debug_log = debug_log_callback;
    parser.syntax = syntax_callback;

    int ret = md_parse(md_text, md_size, &parser, NULL);
    if (ret != 0)
    {
        fprintf(stderr, "Parsing failed with error code: %d\n", ret);
        return ret;
    }

    printf("Parsed HTML:\n%s\n", result);

    return 0;
}