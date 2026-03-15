// SPDX-License-Identifier: Unlicense

#pragma once

#include <stdio.h>

#include <crlib/string.h>
#include <crlib/lambda.h>
#include <crlib/twin.h>

CR_NAMESPACE_BEGIN

// simple stdio file wrapper
class File final : private NonCopyable {
private:
   FILE *handle_ = nullptr;
   size_t length_ {};

public:
   explicit File () = default;

   File (StringRef file, StringRef mode = "rt") {
      open (file, mode);
   }

   ~File () {
      close ();
   }

public:
   bool open (StringRef file, StringRef mode) {
      close ();

      handle_ = plat.openStdioFile (file.chars (), mode.chars ());

      if (!handle_) {
         return false;
      }
      fseek (handle_, 0L, SEEK_END);
      auto pos = ftell (handle_);

      length_ = pos > 0 ? static_cast <size_t> (pos) : 0;
      fseek (handle_, 0L, SEEK_SET);

      return true;
   }

   void close () {
      if (handle_) {
         fclose (handle_);
         handle_ = nullptr;
      }
      length_ = 0;
   }

   bool eof () const {
      return !handle_ || !!feof (handle_);
   }


   bool flush () const {
      return handle_ && fflush (handle_) == 0;
   }

   int get () const {
      return handle_ ? fgetc (handle_) : EOF;
   }

   bool getLine (String &line) {
      int ch = 0;
      SmallArray <char> data (255);

      line.clear ();

      while ((ch = get ()) != EOF && !eof ()) {
         data.push (static_cast <char> (ch));

         if (ch == '\n') {
            break;
         }
      }

      if (!data.empty ()) {
         line.assign (data.data (), data.length ());
      }
      return !line.empty ();
   }

   template <typename ...Args> size_t puts (const char *fmt, Args &&...args) {
      if (!handle_) {
         return 0;
      }
      auto result = fputs (strings.format (fmt, cr::forward <Args> (args)...), handle_);
      return result >= 0 ? static_cast <size_t> (result) : 0;
   }

   bool puts (const char *buffer) {
      if (!handle_) {
         return false;
      }
      return fputs (buffer, handle_) >= 0;
   }

   int putChar (int ch) {
      return handle_ ? fputc (ch, handle_) : EOF;
   }

   size_t read (void *buffer, size_t size, size_t count = 1) {
      return handle_ ? fread (buffer, size, count, handle_) : 0;
   }

   size_t write (const void *buffer, size_t size, size_t count = 1) {
      return handle_ ? fwrite (buffer, size, count, handle_) : 0;
   }

   bool seek (long offset, int origin) {
      if (!handle_) {
         return false;
      }
      if (origin != SEEK_SET && origin != SEEK_CUR && origin != SEEK_END) {
         return false;
      }
      return fseek (handle_, offset, origin) == 0;
   }

   void rewind () {
      if (handle_) {
         ::rewind (handle_);
      }
   }

   size_t length () const {
      return length_;
   }

   explicit operator bool () const {
      return handle_ != nullptr;
   }

public:
   static inline void makePath (const char *path) {
      String buffer (path);

      for (size_t i = 1; i < buffer.length (); ++i) {
         if (buffer.at (i) == *kPathSeparator) {
            buffer.at (i) = kNullChar;
            plat.createDirectory (buffer.chars ());
            buffer.at (i) = *kPathSeparator;
         }
      }
      plat.createDirectory (buffer.chars ());
   }
};

// storage backend for memory-mapped file loading
class MemFileStorage : public Singleton <MemFileStorage> {
public:
   using LoadFunction = Lambda <uint8_t *(const char *, int *)>;
   using FreeFunction = Lambda <void (void *)>;

private:
   LoadFunction loadFun_ {};
   FreeFunction freeFun_ {};

public:
   explicit MemFileStorage () = default;
   ~MemFileStorage () = default;

public:
   void initialize (LoadFunction loader, FreeFunction unloader) {
      loadFun_ = cr::move (loader);
      freeFun_ = cr::move (unloader);
   }

   uint8_t *load (StringRef file, int *size) {
      if (loadFun_) {
         return loadFun_ (file.chars (), size);
      }
      return nullptr;
   }

   void unload (void *buffer) {
      if (freeFun_) {
         freeFun_ (buffer);
      }
   }

public:
   static uint8_t *defaultLoad (const char *path, int *size) {
      File file (path, "rb");

      if (!file) {
         *size = 0;
         return nullptr;
      }
      *size = static_cast <int> (file.length ());
      auto data = mem::allocate <uint8_t> (*size);

      file.read (data, *size);
      return data;
   }

   static void defaultUnload (void *buffer) {
      mem::release (buffer);
   }

   static String loadToString (StringRef filename) {
      int result = 0;
      auto buffer = defaultLoad (filename.chars (), &result);

      if (result > 0 && buffer) {
         String data (reinterpret_cast <char *> (buffer), result);
         defaultUnload (buffer);

         return data;
      }
      return "";
   }
};

// in-memory read-only file
class MemFile final : public NonCopyable {
private:
   uint8_t *contents_ = nullptr;
   size_t length_ {};
   size_t pos_ {};

public:
   explicit MemFile () = default;

   MemFile (StringRef file) {
      open (file);
   }

   ~MemFile () {
      close ();
   }

public:
   bool open (StringRef file) {
      close ();

      int size = 0;
      contents_ = MemFileStorage::instance ().load (file.chars (), &size);
      length_ = size > 0 ? static_cast <size_t> (size) : 0;

      if (!contents_) {
         length_ = 0;
         return false;
      }
      return true;
   }

   void close () {
      if (contents_) {
         MemFileStorage::instance ().unload (contents_);
         contents_ = nullptr;
      }
      length_ = 0;
      pos_ = 0;
   }

   int get () {
      if (!contents_ || pos_ >= length_) {
         return EOF;
      }
      return static_cast <int> (contents_[pos_++]);
   }

   bool getLine (String &line) {
      int ch = 0;
      SmallArray <char> data (255);

      line.clear ();

      while ((ch = get ()) != EOF && !eof ()) {
         data.push (static_cast <char> (ch));

         if (ch == '\n') {
            break;
         }
      }

      if (!data.empty ()) {
         line.assign (data.data (), data.length ());
      }
      return !line.empty ();
   }

   size_t read (void *buffer, size_t size, size_t count = 1) {
      if (!contents_ || pos_ >= length_ || !buffer || !size || !count) {
         return 0;
      }
      auto remaining = length_ - pos_;
      auto requested = size * count;
      auto bytes = requested <= remaining ? requested : remaining;

      memcpy (buffer, &contents_[pos_], bytes);
      pos_ += bytes;

      return bytes / size;
   }

   bool seek (size_t offset, int origin) {
      if (!contents_) {
         return false;
      }

      switch (origin) {
      case SEEK_SET:
         if (offset >= length_) {
            return false;
         }
         pos_ = offset;
         break;

      case SEEK_END:
         if (offset >= length_) {
            return false;
         }
         pos_ = length_ - offset;
         break;

      case SEEK_CUR:
         if (pos_ + offset >= length_) {
            return false;
         }
         pos_ += offset;
         break;

      default:
         return false;
      }
      return true;
   }

   void rewind () {
      pos_ = 0;
   }

   size_t length () const {
      return length_;
   }

   bool eof () const {
      return pos_ >= length_;
   }

   explicit operator bool () const {
      return contents_ != nullptr && length_ > 0;
   }
};

namespace detail {
   struct FileEnumeratorEntry : public NonCopyable {
      FileEnumeratorEntry () = default;
      ~FileEnumeratorEntry () = default;

#if defined(CR_WINDOWS)
      bool next {};
      String path {};
      HANDLE handle = INVALID_HANDLE_VALUE;
      WIN32_FIND_DATAA data {};
#else
      String path {};
      String mask {};
      DIR *dir {};
      dirent *entry {};
#endif
   };
}

// file enumerator
class FileEnumerator : public NonCopyable {
private:
   UniquePtr <detail::FileEnumeratorEntry> entry_;

public:
   FileEnumerator (StringRef mask) : entry_ (cr::makeUnique <detail::FileEnumeratorEntry> ()) {
      start (mask);
   }

   ~FileEnumerator () {
      close ();
   }

public:
   void start (StringRef mask) {
      auto sep = mask.findLastOf (kPathSeparator);

      if (sep != StringRef::InvalidIndex) {
         entry_->path = mask.substr (0, sep);
      }
      else {
         entry_->path = ".";
      }

#if defined(CR_WINDOWS)
      entry_->handle = FindFirstFileA (mask.chars (), &entry_->data);
      entry_->next = entry_->handle != INVALID_HANDLE_VALUE;
#else
      entry_->mask = sep != StringRef::InvalidIndex ? mask.substr (sep + 1) : mask;
      entry_->dir = opendir (entry_->path.chars ());

      if (entry_->dir) {
         next ();
      }
#endif
   }

   void close () {
#if defined(CR_WINDOWS)
      if (entry_->handle != INVALID_HANDLE_VALUE) {
         FindClose (entry_->handle);

         entry_->handle = INVALID_HANDLE_VALUE;
         entry_->next = false;
      }
#else
      if (entry_->dir) {
         closedir (entry_->dir);
         entry_->dir = nullptr;
      }
#endif
   }

   bool next () {
#if defined(CR_WINDOWS)
      entry_->next = !!FindNextFileA (entry_->handle, &entry_->data);
      return entry_->next;
#else
      while ((entry_->entry = readdir (entry_->dir)) != nullptr) {
         if (!fnmatch (entry_->mask.chars (), entry_->entry->d_name, FNM_CASEFOLD | FNM_NOESCAPE | FNM_PERIOD)) {
            return true;
         }
      }
      return false;
#endif
   }

   String getMatch () const {
      StringRef match {};

#if defined(CR_WINDOWS)
      match = entry_->data.cFileName;
#else
      match = entry_->entry->d_name;
#endif
      return String::join ({ entry_->path, match }, kPathSeparator);
   }

   operator bool () const {
#if defined(CR_WINDOWS)
      return entry_->next;
#else
      return entry_->dir != nullptr && entry_->entry != nullptr;
#endif
   }
};

CR_NAMESPACE_END
