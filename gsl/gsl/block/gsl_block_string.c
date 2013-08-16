#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <gsl/gsl_errno.h>
#include "gsl_block_string.h"

gsl_block_string *
gsl_block_string_alloc (const size_t n)
{
    gsl_block_string * b;
      
    b = (gsl_block_string *) malloc (sizeof (gsl_block_string));
    
    if (b == 0)
    {
        GSL_ERROR_VAL ("failed to allocate space for block struct",
                       GSL_ENOMEM, 0);
    }
    
    if (n > 0)
    {
        b->data = (string *) calloc (1, 1 * n * sizeof (string));
        
        if (b->data == 0)
        {
            free (b);
            
            GSL_ERROR_VAL ("failed to allocate space for block data",
                           GSL_ENOMEM, 0);
        }
        for (size_t i = 0; i < n; i++) {
            b->data[i] = NULL;
        }
    }
    else
    {
        b->data = NULL;
    }
    b->size = n;
    
    return b;
}

gsl_block_string *
gsl_block_string_calloc (const size_t n)
{
  size_t i;

  gsl_block_string * b = gsl_block_string_alloc (n);

  if (b == 0)
    return 0;

  for (i = 0; i < 1 * n; i++)
    {
      b->data[i] = 0;
    }

  return b;
}

void
gsl_block_string_free (gsl_block_string * b)
{
    if (!b) { return ; };
    for (size_t i = 0; i < b->size; i++)
    {
        if (b->data[i] != 0)
        {
            free(b->data[i]);
        }
    }
    free (b->data);
    free (b);
}

int
gsl_block_string_fread (FILE * stream, gsl_block_string * b)
{
    return gsl_block_string_raw_fread(stream, b->data, b->size, 1);
}

int
gsl_block_string_fwrite (FILE * stream, const gsl_block_string * b)
{
    return gsl_block_string_raw_fwrite(stream, b->data, b->size, 1);
}

int
gsl_block_string_raw_fread (FILE * stream, string * data,
                            const size_t n, const size_t stride)
{
    if (stride == 0)
    {
        GSL_ERROR ("stride must be positive integer", GSL_EINVAL);
    }
    
    for (size_t i = 0; i < n; i++) {
        size_t lengthOfString;
        size_t items = fread (&lengthOfString, 1 * sizeof (size_t), 1, stream);
        if (items != 1)
        {
            GSL_ERROR ("fread failed", GSL_EFAILED);
        }
        if (data[i * stride] != NULL) {
            free(data[i * stride]);
        }
        data[i * stride] = malloc((lengthOfString + 1)*sizeof(char));
        if (data[i * stride] == NULL)
        {
            GSL_ERROR ("fread failed", GSL_ENOMEM);
        }
        if (lengthOfString > 0)
        {
            items = fread (data[i * stride], 1 * sizeof (char), lengthOfString, stream);
            if (items != lengthOfString)
            {
                GSL_ERROR ("fread failed", GSL_EFAILED);
            }
        }
        data[i * stride][lengthOfString] = '\0';
    }
        
    return GSL_SUCCESS;
}


int
gsl_block_string_raw_fwrite (FILE * stream, const string * data,
                             const size_t n, const size_t stride)
{
    if (stride == 0)
    {
        GSL_ERROR ("stride must be positive integer", GSL_EINVAL);
    }
    for (size_t i = 0; i < n; i++) {
        char *p = data[i * stride];
        size_t lengthOfString;
        if (p == NULL)
        {
            lengthOfString = 0;
        }
        else
        {
            lengthOfString = strlen(p);
        }
        size_t items = fwrite (&lengthOfString, 1 * sizeof (size_t), 1, stream);
        if (items != 1)
        {
            GSL_ERROR ("fwrite failed", GSL_EFAILED);
        }
        if (p != NULL)
        {
            items = fwrite (p, 1 * sizeof (char), lengthOfString, stream);
            if (items != lengthOfString)
            {
                GSL_ERROR ("fwrite failed", GSL_EFAILED);
            }
        }
    }
    
    return GSL_SUCCESS;
}

int
gsl_block_string_fprintf (FILE * stream, const gsl_block_string * b, const char *format)
{
    return gsl_block_string_raw_fprintf(stream, b->data, b->size, 1, format);
}

int
gsl_block_string_fscanf (FILE * stream, gsl_block_string * b)
{
    return gsl_block_string_raw_fscanf(stream, b->data, b->size, 1);
}

int
gsl_block_string_raw_fprintf (FILE * stream,
                              const string * data,
                              const size_t n,
                              const size_t stride,
                              const char *format)
{
    size_t i;
    assert(stride == 1);
    
    for (i = 0; i < n; i++)
    {
        int status;
        
        char *p = data[i * stride];
        size_t lengthOfString;
        if (p == NULL)
        {
            lengthOfString = 0;
        }
        else
        {
            lengthOfString = strlen(p);
        }
        if (p != NULL)
        {
            status = fprintf (stream,
                              format,
                              data[1 * i * stride]);
            if (status < 0)
            {
                GSL_ERROR ("fprintf failed", GSL_EFAILED);
            }
        }
        
        status = putc ('\n', stream);
        
        if (status == (-1))
        {
            GSL_ERROR ("putc failed", GSL_EFAILED);
        }
    }
    
    return 0;
}

#ifndef BUFSIZE
#define BUFSIZE 2
#endif

int
gsl_block_string_raw_fscanf (FILE * stream,
                             string * data,
                             const size_t n,
                             const size_t stride)
{
    char *t;
    size_t i;
    
    char tmp[BUFSIZE + 1];

    for (i = 0; i < n; i++)
    {
        int k;
        for (k = 0; k < 1; k++)
        {
            char c;
            size_t j = 0;
            size_t jj = 0;
            c = fgetc (stream);
            if (feof(stream))
            {
                GSL_ERROR ("fscanf failed", GSL_EFAILED);
            }
            while (isspace(c) && !feof(stream)) {
                c = fgetc (stream);
            }
            if (feof(stream))
            {
                GSL_ERROR ("fscanf failed", GSL_EFAILED);
            }
            while (!isspace(c)) {
                tmp[j++] = c;
                if (j == BUFSIZE)
                {
                    j = 0;
                    jj++;
                    t = realloc(data [1 * i * stride + k], (BUFSIZE * jj + 1)*sizeof(char));
                    if (t == NULL)
                    {
                        GSL_ERROR("fscanf failed", GSL_ENOMEM);
                    }
                    data [1 * i * stride + k] = t;
                    data [1 * i * stride + k][(jj - 1)*BUFSIZE] = '\0';
                    tmp[BUFSIZE] = '\0';
                    strcat(data [1 * i + k], tmp);
                }
                c = fgetc (stream);
            }
            if (j == 0 && jj == 0)
            {
                GSL_ERROR ("fscanf failed", GSL_EFAILED);
            }
            tmp[j] = '\0';
            size_t lengthOfTmp = strlen(tmp);
            t = realloc(data [1 * i * stride + k], (BUFSIZE * jj + lengthOfTmp + 1)*sizeof(char));
            if (t == NULL)
            {
                GSL_ERROR("fscanf failed", GSL_ENOMEM);
            }
            data [1 * i * stride + k] = t;
            data [1 * i * stride + k][jj*BUFSIZE] = '\0';
            strcat(data [1 * i * stride + k], tmp);
        }
    }
    
    return GSL_SUCCESS;
}


size_t
gsl_block_string_size (const gsl_block_string * b)
{
  return b->size ;
}

string *
gsl_block_string_data (const gsl_block_string * b)
{
  return b->data ;
}

int
gsl_block_string_add_string (gsl_block_string * b, const string a)
{
    bool isAdded = false;
    if (a == NULL)
    {
        GSL_ERROR("string must not be null", GSL_EINVAL);
    }
    size_t lengthOfA = strlen(a);
    for (size_t i = 0; i < b->size; i++) {
        if (b->data[i] == NULL)
        {
            b->data[i] = malloc((lengthOfA + 1) * sizeof(char));
            if (b->data[i] == NULL)
            {
                GSL_ERROR("adding string failed in block", GSL_ENOMEM);
            }
            strcpy(b->data[i], a);
            isAdded = true;
            break;
        }
    }
    if (isAdded == false)
    {
        string *t = realloc(b->data, (b->size + 1) * sizeof(string));
        if (t == NULL)
        {
            GSL_ERROR("adding string failed in block", GSL_ENOMEM);
        }
        b->data = t;
        b->data[b->size] = malloc((lengthOfA + 1) * sizeof(char));
        if (b->data[b->size] == NULL)
        {
            GSL_ERROR("adding string failed in block", GSL_ENOMEM);
        }
        strcpy(b->data[b->size], a);
        b->size++;
    }
    return GSL_SUCCESS;
}

gsl_block_string *gsl_block_string_realloc (gsl_block_string * b, const size_t n)
{
    if (b->size < n)
    {
        string *t;
        t = realloc(b->data, n * sizeof(string));
        if (t == NULL)
        {
            GSL_ERROR_VAL ("failed to re-allocate space for block struct",
                           GSL_ENOMEM, 0);
        }
        b->data = t;
        for (size_t i = n; i < b->size; i++)
        {
            b->data[i] = NULL;
        }
    }
    else
    {
        for (size_t i = n; i < b->size; i++)
        {
            if (b->data[i] != NULL) {
                free(b->data[i]);
            }
        }
        string *t;
        t = realloc(b->data, n * sizeof(string));
        if (t == NULL)
        {
            GSL_ERROR_VAL ("failed to re-allocate space for block struct",
                           GSL_ENOMEM, 0);
        }
        b->data = t;
    }
    b->size = n;
    return b;
}

