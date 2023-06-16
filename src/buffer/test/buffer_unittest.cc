#include "buffer.h"

//#define BOOST_TEST_MODULE BufferTest
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using std::string;

BOOST_AUTO_TEST_CASE(testBufferAppendRetrieve)
{
  Buffer buf;
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend);

  const string str(200, 'x');
  buf.Append(str);
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), str.size());
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize - str.size());
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend);

  const string str2 =  buf.RetrieveAsString(50);
  BOOST_CHECK_EQUAL(str2.size(), 50);
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), str.size() - str2.size());
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize - str.size());
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend + str2.size());
  BOOST_CHECK_EQUAL(str2, string(50, 'x'));

  buf.Append(str);
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 2*str.size() - str2.size());
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize - 2*str.size());
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend + str2.size());

  const string str3 =  buf.RetrieveAllAsString();
  BOOST_CHECK_EQUAL(str3.size(), 350);
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend);
  BOOST_CHECK_EQUAL(str3, string(350, 'x'));
}

BOOST_AUTO_TEST_CASE(testBufferGrow)
{
  Buffer buf;
  buf.Append(string(400, 'y'));
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 400);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize-400);

  buf.Retrieve(50);
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 350);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize-400);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend+50);

  buf.Append(string(1000, 'z'));
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 1350);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend+50); // FIXME

  buf.RetrieveAll();
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), 1400); // FIXME
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend);
}

BOOST_AUTO_TEST_CASE(testBufferInsideGrow)
{
  Buffer buf;
  buf.Append(string(800, 'y'));
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 800);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize-800);

  buf.Retrieve(500);
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 300);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize-800);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend+500);

  buf.Append(string(300, 'z'));
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 600);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize-600);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend);
}



BOOST_AUTO_TEST_CASE(testBufferPrepend)
{
  Buffer buf;
  buf.Append(string(200, 'y'));
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 200);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize-200);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend);

  int x = 0;
  buf.Prepend(&x, sizeof x);
  BOOST_CHECK_EQUAL(buf.ReadableBytes(), 204);
  BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize-200);
  BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend - 4);
}




void output(Buffer&& buf, const void* inner)
{
  Buffer newbuf(std::move(buf));
  // printf("New Buffer at %p, inner %p\n", &newbuf, newbuf.Peek());
  BOOST_CHECK_EQUAL(inner, newbuf.Peek());
}

// NOTE: This test fails in g++ 4.4, passes in g++ 4.6.
BOOST_AUTO_TEST_CASE(testMove)
{
  Buffer buf;
  buf.Append("muduo", 5);
  const void* inner = buf.Peek();
  // printf("Buffer at %p, inner %p\n", &buf, inner);
  output(std::move(buf), inner);
}
