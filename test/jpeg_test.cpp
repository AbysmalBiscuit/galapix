#include <gtest/gtest.h>

#include "math/size.hpp"
#include "plugins/jpeg.hpp"

TEST(JPEGTest, get_size)
{
  EXPECT_EQ(Size(64, 128), JPEG::get_size("test/data/images/rgb.jpg"));
  EXPECT_EQ(Size(64, 128), JPEG::get_size("test/data/images/grayscale.jpg"));
  EXPECT_EQ(Size(64, 128), JPEG::get_size("test/data/images/cmyk.jpg"));
}

TEST(JPEGTest, load_from_file)
{
  EXPECT_NO_THROW({ JPEG::load_from_file("test/data/images/rgb.jpg"); });
  EXPECT_NO_THROW({ JPEG::load_from_file("test/data/images/grayscale.jpg"); });
  EXPECT_NO_THROW({ JPEG::load_from_file("test/data/images/cmyk.jpg"); });
}

/* EOF */
