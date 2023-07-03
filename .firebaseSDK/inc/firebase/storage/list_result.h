#ifndef FIREBASE_STORAGE_SRC_INCLUDE_FIREBASE_STORAGE_LIST_RESULT_H_
#define FIREBASE_STORAGE_SRC_INCLUDE_FIREBASE_STORAGE_LIST_RESULT_H_

#include <string>
#include <vector>

namespace firebase {
namespace storage {
namespace internal {
class StorageInternal;
}
class StorageReference;

class ListResult {
 public:
  static const char* kItemsKey;
  static const char* kNameKey;
  static const char* kPageTokenKey;
  static const char* kPrefixesKey;

  bool ImportFromJson(std::string& json, internal::StorageInternal* storage);

  std::vector<StorageReference> GetPrefixes() const { return prefixes_; }

  std::vector<StorageReference> GetItems() const { return items_; }

  std::string GetPageToken() const { return pageToken_; }

 private:
  std::vector<StorageReference> prefixes_;
  std::vector<StorageReference> items_;
  std::string pageToken_;
};
}  // namespace storage
}  // namespace firebase

#endif  // FIREBASE_STORAGE_SRC_INCLUDE_FIREBASE_STORAGE_LIST_RESULT_H_
