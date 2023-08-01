#include "buffer/buffer.h"
#include <gtest/gtest.h>

using std::string;

class BufferTest : public ::testing::Test {
protected:
    Buffer buf;
    const size_t kCheapPrepend = Buffer::kCheapPrepend;
    const size_t kInitialSize = Buffer::kInitialSize;
};

TEST_F(BufferTest, testBufferAppendRetrieve) {
    const string str(200, 'x');
    buf.Append(str);
    EXPECT_EQ(buf.ReadableBytes(), str.size());
    EXPECT_EQ(buf.WritableBytes(), kInitialSize - str.size());
    EXPECT_EQ(buf.PrependableBytes(), kCheapPrepend);

    const string str2 = buf.RetrieveAsString(50);
    EXPECT_EQ(str2.size(), 50);
    EXPECT_EQ(buf.ReadableBytes(), str.size() - str2.size());
    EXPECT_EQ(buf.WritableBytes(), kInitialSize - str.size());
    EXPECT_EQ(buf.PrependableBytes(), kCheapPrepend + str2.size());
    EXPECT_EQ(str2, string(50, 'x'));

    buf.Append(str);
    EXPECT_EQ(buf.ReadableBytes(), 2 * str.size() - str2.size());
    EXPECT_EQ(buf.WritableBytes(), kInitialSize - 2 * str.size());
    EXPECT_EQ(buf.PrependableBytes(), kCheapPrepend + str2.size());

    const string str3 = buf.RetrieveAllAsString();
    EXPECT_EQ(str3.size(), 350);
    EXPECT_EQ(buf.ReadableBytes(), 0);
    EXPECT_EQ(buf.WritableBytes(), kInitialSize);
    EXPECT_EQ(buf.PrependableBytes(), kCheapPrepend);
    EXPECT_EQ(str3, string(350, 'x'));
}

TEST_F(BufferTest, testBufferGrow) {
    buf.Append(string(400, 'y'));
    EXPECT_EQ(buf.ReadableBytes(), 400);
    EXPECT_EQ(buf.WritableBytes(), kInitialSize - 400);

    buf.Retrieve(50);
    EXPECT_EQ(buf.ReadableBytes(), 350);
    EXPECT_EQ(buf.WritableBytes(), kInitialSize - 400);
    EXPECT_EQ(buf.PrependableBytes(), kCheapPrepend + 50);

    buf.Append(string(1000, 'z'));
    EXPECT_EQ(buf.ReadableBytes(), 1350);
    EXPECT_EQ(buf.WritableBytes(), 0);
    EXPECT_EQ(buf.PrependableBytes(), kCheapPrepend + 50); 

    buf.RetrieveAll();
    EXPECT_EQ(buf.ReadableBytes(), 0);
    EXPECT_EQ(buf.WritableBytes(), 1400);  
    EXPECT_EQ(buf.PrependableBytes(), kCheapPrepend);
}

TEST_F(BufferTest, testBufferInsideGrow) {
    buf.Append(string(800, 'y'));
    EXPECT_EQ(buf.ReadableBytes(), 800);
    EXPECT_EQ(buf.WritableBytes(), kInitialSize - 800);

    buf.Retrieve(500);
    EXPECT_EQ(buf.ReadableBytes(), 300);
    EXPECT_EQ(buf.WritableBytes(), kInitialSize - 800);
    EXPECT_EQ(buf.PrependableBytes(), kCheapPrepend + 500);

    buf.Append(string(300, 'z'));
    EXPECT_EQ(buf.ReadableBytes(), 600);
    EXPECT_EQ(buf.WritableBytes(), kInitialSize - 600);
    EXPECT_EQ(buf.PrependableBytes(), kCheapPrepend);
}

TEST_F(BufferTest, testBufferPrepend) {
    buf.Append(string(200, 'y'));
    EXPECT_EQ(buf.ReadableBytes(), 200);
    EXPECT_EQ(buf.WritableBytes(), kInitialSize - 200);
    EXPECT_EQ(buf.PrependableBytes(), kCheapPrepend);

    int x = 0;
    buf.Prepend(&x, sizeof x);
    EXPECT_EQ(buf.ReadableBytes(), 204);
    EXPECT_EQ(buf.WritableBytes(), kInitialSize - 200);
    EXPECT_EQ(buf.PrependableBytes(), kCheapPrepend - 4);
}

TEST_F(BufferTest, testBufferFindEOL) {
    buf.Append(string(100000, 'x'));
    const char* null = nullptr;
    EXPECT_EQ(buf.FindEOL(), null);
    EXPECT_EQ(buf.FindEOL(buf.Peek() + 90000), null);
}

void output(Buffer&& buf, const void* inner) {
    Buffer newbuf(std::move(buf));
    EXPECT_EQ(inner, newbuf.Peek());
}

TEST_F(BufferTest, testMove) {
    buf.Append("muduo", 5);
    const void* inner = buf.Peek();
    output(std::move(buf), inner);
}
