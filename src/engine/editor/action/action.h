#ifndef PRT3_ACTION_H
#define PRT3_ACTION_H

namespace prt3 {

class EditorContext;

class Action {
public:
    virtual ~Action() {}

    virtual bool apply() = 0;
    virtual bool unapply() = 0;
};

} // namespace prt3

#endif
