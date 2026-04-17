#pragma once

#include <cerrno>
#include <iconv.h>
#include <string>

namespace yi::serialize::binary::detail
{

// 通用编码转换（POSIX iconv）。转换失败返回 false。
inline bool transcode(const char* from_enc, const char* to_enc,
											const char* in_data, size_t in_len, std::string& out)
{
	iconv_t cd = iconv_open(to_enc, from_enc);
	if (cd == (iconv_t)-1)
		return false;

	// 最坏情况：GBK→UTF-8 每字节最多扩展为 3 字节
	out.resize(in_len * 3);

	char*  inbuf   = const_cast<char*>(in_data);
	size_t inleft  = in_len;
	char*  outbuf  = out.data();
	size_t outleft = out.size();

	while (inleft > 0) 
	{
		size_t r = iconv(cd, &inbuf, &inleft, &outbuf, &outleft);
		if (r == (size_t)-1) 
		{
			if (errno == E2BIG) 
			{
				// 输出缓冲不足，翻倍扩容
				size_t done = out.size() - outleft;
				out.resize(out.size() * 2);
				outbuf  = out.data() + done;
				outleft = out.size() - done;
			}
			else 
			{
				iconv_close(cd);
				return false;
			}
		}
	}

	out.resize(out.size() - outleft);
	iconv_close(cd);
	return true;
}

// 内部 UTF-8 → 外部 GBK
inline bool utf8_to_gbk(const std::string& utf8, std::string& gbk)
{
	if (utf8.empty())
	{ 
		gbk.clear(); 
		return true; 
	}
	return transcode("UTF-8", "GBK", utf8.data(), utf8.size(), gbk);
}

// 外部 GBK → 内部 UTF-8
inline bool gbk_to_utf8(const char* data, size_t len, std::string& utf8)
{
	if (len == 0) 
	{ 
		utf8.clear(); 
		return true; 
	}
	return transcode("GBK", "UTF-8", data, len, utf8);
}

} // namespace yi::serialize::binary::detail
