#ifndef PRT3_ACTION_H
#define PRT3_ACTION_H

namespace prt3 {

class ActionManager;

class EditorContext;

class Action {
public:
    virtual ~Action() {}

protected:
    virtual bool apply() = 0;
    virtual bool unapply() = 0;

    friend class ActionManager;
};

} // namespace prt3

#endif
