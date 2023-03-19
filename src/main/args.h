#ifndef PRT3_ARGS_H
#define PRT3_ARGS_H

#include <string>

void parse_args(int, char**);

namespace prt3 {

class Args
{
private:
    std::string m_project_path;
    bool m_force_cached = false;

   Args() {}

    static Args & instance()
    {
        static Args INSTANCE;
        return INSTANCE;
    }

public:
   inline static std::string const & project_path()
   { return instance().m_project_path; }

   inline static bool force_cached()
   { return instance().m_force_cached; }

   friend void ::parse_args(int, char**);
};

} // namespace prt3

#endif // PRT3_ARGS_H
