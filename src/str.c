#include <str.h>

#include <stdarg.h>

STR STR_Allocate(MEM_Arena *arena, UZ size)
{
  STR string = { .str = MEM_ArenaAllocate(arena, size + 1) };
  if (string.str)
  {
    string.str[size] = 0;
    string.size = size;
  }
  return string;
}

STR STR_Copy(MEM_Arena *arena, STR other)
{
  STR string = { .str = MEM_ArenaAllocate(arena, other.size + 1) };
  if (string.str)
  {
    MemoryCopy(string.str, other.str, other.size);
    string.str[string.size = other.size] = 0;
  }
  return string;
}

STR STR_Cat(MEM_Arena *arena, STR left, STR right)
{
  STR string = STR_Allocate(arena, left.size + right.size);
  if (string.str)
  {
    MemoryCopy(string.str, left.str, left.size);
    MemoryCopy(string.str + left.size, right.str, right.size);
  }
  return string;
}

STR STR_Replace(MEM_Arena *arena, STR string, STR substring, STR replacement)
{
  UZ count = STR_Count(string, substring);
  STR result = STR_Allocate(arena, string.size - substring.size * count + replacement.size * count);
  for (UZ i = 0, j = 0; i < result.size && j < string.size;)
  {
    UZ found = STR_FindFirst(string, substring, i);
    if (found >= string.size)
    {
      MemoryCopy(result.str + j, string.str + i, string.size - i);
      break;
    }
    MemoryCopy(result.str + j, string.str + i, found - i);
    j += found - i;
    MemoryCopy(result.str + j, replacement.str, replacement.size);
    j += replacement.size;
    i = found + substring.size;
  }
  return result;
}

UZ STR_Count(STR string, STR substring)
{
  UZ count = 0;
  for (UZ i = 0; i < string.size; ++i)
  {
    i = STR_FindFirst(string, substring, i);
    if (i < string.size) ++count;
  }
  return count;
}

UZ STR_FindFirst(STR string, STR substring, UZ offset)
{
  if (string.size < substring.size) return string.size;
  while (offset < string.size)
  {
    if (offset < string.size - substring.size)
    {
      STR slice = { .str = string.str + offset, .size = substring.size };
      if (STR_Equals(slice, substring)) break;
    }
    else offset = string.size;
  }
  return offset;
}

UZ STR_FindLast(STR string, STR substring, UZ offset)
{
  return string.size; /* TODO: implement STR_FindLast */
}

U32 STR_Hash(STR string)
{
  U32 hash = 0;
  for (UZ i = 0; i < string.size; i++)
  {
    hash = (hash << 5) - hash + string.str[i];
  }
  return hash;
}

U64 STR_Hash64(STR string)
{
  U64 hash = 0;
  for (UZ i = 0; i < string.size; i++)
  {
    hash = (hash << 5) - hash + string.str[i];
  }
  return hash;
}

bool STR_Equals(STR left, STR right)
{
  bool result = (left.size == right.size);
  if (left.str != right.str)
  {
    for (UZ i = 0; i < left.size && result; ++i)
    {
      result = (left.str[i] == left.str[i]);
    }
  }
  return result;
}

STR16 STR16_Make(U16 *s)
{
  STR16 string = { .str = s };
  if (s) while (s[string.size]) ++string.size;
  return string;
}

STR16 STR16_Allocate(MEM_Arena *arena, UZ size)
{
  STR16 string = { .str = MEM_ArenaAllocate(arena, (size + 1) << 1) };
  if (string.str)
  {
    string.str[size] = 0;
    string.size = size;
  }
  return string;
}

STR16 STR16_From_STR(MEM_Arena *arena, STR string)
{
  STR16 result = STR16_Allocate(arena, string.size);
  UZ size = 0;
  if (result.str)
  {
    for (UZ i = 0; i < string.size; ++i)
    {
      STR slice = { .str = string.str + i, .size = string.size - i };
      S32 codepoint = UTF8_DecodeFirst(slice);
      UZ count = UTF16_Encode(result.str + size, result.size - size, codepoint);
      if (!count)
      {
        MEM_ArenaDeallocate(arena, result.str);
        return (STR16) { 0 };
      }
      size += count;
      if (codepoint > 0x7F) ++i;
      if (codepoint > 0x7FF) ++i;
      if (codepoint > 0xFFFF) ++i;
    }
    MEM_ArenaDeallocateSize(arena, (result.size - size) << 1);
    result.str[size] = 0;
    result.size = size;
  }
  return result;
}

STR STR_From_STR16(MEM_Arena *arena, STR16 string)
{
  STR result = STR_Allocate(arena, string.size * 3);
  UZ size = 0;
  if (result.str)
  {
    for (UZ i = 0; i < string.size; ++i)
    {
      STR16 slice = { .str = string.str + i, .size = string.size - i };
      S32 codepoint = UTF16_DecodeFirst(slice);
      UZ count = UTF8_Encode(result.str + size, result.size - size, codepoint);
      if (!count)
      {
        MEM_ArenaDeallocate(arena, result.str);
        return (STR) { 0 };
      }
      size += count;
      if (codepoint > 0xFFFF) ++i;
    }
    MEM_ArenaDeallocateSize(arena, result.size - size);
    result.str[size] = 0;
    result.size = size;
  }
  return result;
}

static U8 UTF8_CharSize[] =
{
  1, 1, 1, 1, 1, 1, 1, 1, /* 00xxx */
  1, 1, 1, 1, 1, 1, 1, 1, /* 01xxx */
  0, 0, 0, 0, 0, 0, 0, 0, /* 10xxx */
  2, 2, 2, 2, 3, 3, 4, 0  /* 11xxx */
};
static U8 UTF8_FirstMask[] = { 0, 0x7F, 0x1F, 0x0F, 0x07 };

S32 UTF8_DecodeFirst(STR string)
{
  S32 codepoint = -1;
  if (string.str && string.size)
  {
    U8 byte = string.str[0];
    U8 size = UTF8_CharSize[byte >> 3];
    if (size <= string.size)
    {
      codepoint = byte & UTF8_FirstMask[size];
      for (U8 i = 1; i < size; ++i)
      {
        if (string.str[i] & 0xC0 != 0x80) return -1;
        codepoint = (codepoint << 6) | (string.str[i] & 0x3F);
      }
    }
  }
  return codepoint;
}

UZ UTF8_Encode(U8 *buffer, UZ capacity, S32 codepoint)
{
  UZ size = 0;
  if (buffer && codepoint > 0)
  {
    if (codepoint < 0x80)
    {
      if (capacity > 0)
      {
        buffer[0] = codepoint;
        size = 1;
      }
    }
    else if (codepoint < 0x800)
    {
      if (capacity > 1)
      {
        buffer[0] = 0xC0 | (codepoint >> 6);
        buffer[1] = 0x80 | (codepoint & 0x3F);
        size = 2;
      }
    }
    else if (codepoint < 0x10000)
    {
      if (capacity > 2)
      {
        buffer[0] = 0xE0 | (codepoint >> 12);
        buffer[1] = 0x80 | ((codepoint >> 6) & 0x3F);
        buffer[2] = 0x80 | (codepoint & 0x3F);
        size = 3;
      }
    }
    else if (codepoint < 0x200000)
    {
      if (capacity > 3)
      {
        buffer[0] = 0xF0 | (codepoint >> 18);
        buffer[1] = 0x80 | ((codepoint >> 12) & 0x3F);
        buffer[2] = 0x80 | ((codepoint >> 6) & 0x3F);
        buffer[3] = 0x80 | (codepoint & 0x3F);
        size = 4;
      }
    }
  }
  return size;
}

UZ UTF8_GetLength(STR string)
{
  UZ length = 0;
  for (UZ i = 0; i < string.size; ++i)
  {
    STR slice = { .str = string.str + i, .size = string.size - i };
    S32 codepoint = UTF8_DecodeFirst(slice);
    if (codepoint < 0) return 0;
    if (codepoint > 0x7F) ++i;
    if (codepoint > 0x7FF) ++i;
    if (codepoint > 0xFFFF) ++i;
    ++length;
  }
  return length;
}

S32 UTF16_DecodeFirst(STR16 string)
{
  U32 codepoint = -1;
  U16 high = string.str[0];
  switch (high & 0xDC00)
  {
  case 0xD800:
    if (string.size > 1)
    {
      U16 low = string.str[1];
      if ((low & 0xDC00) == 0xDC00)
      {
        U32 offset = (low & 0x3FF) | ((high & 0x3FF) << 10);
        codepoint = 0x10000 + offset;
      }
    }
  case 0xDC00:
    break;
  default:
    codepoint = high;
    break;
  }
  return codepoint;
}

UZ UTF16_Encode(U16 *buffer, UZ capacity, S32 codepoint)
{
  UZ size = 0;
  if (buffer && codepoint > 0)
  {
    if (codepoint < 0x10000)
    {
      if (capacity > 0 && (codepoint & 0xD800) != 0xD800)
      {
        buffer[0] = codepoint;
        size = 1;
      }
    }
    else if (codepoint < 0x110000)
    {
      if (capacity > 1)
      {
        buffer[0] = 0xD800 | (codepoint >> 10);
        buffer[1] = 0xDC00 | (codepoint & 0x3FF);
        size = 2;
      }
    }
  }
  return size;
}

UZ UTF16_GetLength(STR16 string)
{
  UZ length = 0;
  for (UZ i = 0; i < string.size; ++i)
  {
    STR16 slice = { .str = string.str + i, .size = string.size - i };
    S32 codepoint = UTF16_DecodeFirst(slice);
    if (codepoint < 0) return 0;
    if (codepoint > 0xFFFF) ++i;
    ++length;
  }
  return length;
}
