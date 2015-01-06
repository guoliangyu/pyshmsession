
namespace PyShmSession
{

bool init();

bool get(const char* key, int klen, unsigned char* data, int len);

bool add(const char* key, int klen, unsigned char* data, int dlen);

bool update(const char* key, int klen, unsigned char* data, int dlen);

void remove(const char* key, int klen);

bool touch(const char* key, int klen);

void recycle(int ncycles);

int size();

}

