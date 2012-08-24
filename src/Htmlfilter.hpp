#include <boost/iostreams/concepts.hpp>    // output_filter
#include <boost/iostreams/operations.hpp>  // put

class Htmlfilter : public boost::iostreams::output_filter {
public:
	template<typename Sink>
	bool put(Sink& dest, int c);
};

template<typename Sink>
bool Htmlfilter::put(Sink& dest, int c)
{
	namespace io = boost::iostreams;
	switch (c)
	{
		case '<': {
			static const char lt[] = "&lt;";
			return io::write(dest, lt, sizeof lt - 1) == sizeof lt - 1;
		} break;
		case '>': {
			static const char rt[] = "&gt;";
			return io::write(dest, rt, sizeof rt - 1) == sizeof rt - 1;
		} break;
		case '&': {
			static const char amp[] = "&amp;";
			return io::write(dest, amp, sizeof amp - 1) == sizeof amp - 1;
		} break;
		case '"': {
			static const char quot[] = "&quot;";
			return io::write(dest, quot, sizeof quot - 1) == sizeof quot - 1;
		} break;
		case '\n': {
			static const char newline[] = "<br/>";
			return io::write(dest, newline, sizeof newline - 1) == sizeof newline - 1;
		} break;
		default: return io::put(dest, static_cast<char>(c));
	}
}