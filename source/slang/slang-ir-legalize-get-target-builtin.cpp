#include "slang-ir-legalize-get-target-builtin.h"

#include "slang-ir-call-graph.h"
#include "slang-ir-insts.h"


namespace Slang
{

class LegalizeGetTargetBuiltinContext
{
public:
    explicit LegalizeGetTargetBuiltinContext(IRModule* module)
        : m_module(module), m_builder(m_module)
    {
        for (auto inst : m_module->getGlobalInsts())
        {
            if (auto getTargetBuiltinInst = as<IRGetTargetBuiltin>(inst))
            {
                m_getTargetBuiltinInsts.add(getTargetBuiltinInst);
            }
        }

        printf("FW: NUM TARGET builtin insts: %d\n", m_getTargetBuiltinInsts.getCount());

        if (m_getTargetBuiltinInsts.getCount() > 0)
        {
            buildEntryPointReferenceGraph(m_entryPointReferenceGraph, m_module);
        }
    }

    void legalize()
    {
        int i = 0;
        // return;
        for (auto inst : m_getTargetBuiltinInsts)
        {
            // if (i == 1 || i == 2)
            // if (i == 1)
            {
                legalizeInst(inst);
            }
            ++i;
        }
    }

private:
    void legalizeInst(IRGetTargetBuiltin* getTargetBuiltinInst)
    {
        // The get target builtin instruction is replaced by a global-scope variable.
        auto builtinType = getTargetBuiltinInst->getFullType();
        auto globalVar = m_builder.createGlobalVar(builtinType);

        for (auto use = getTargetBuiltinInst->firstUse; use != nullptr; use = use->nextUse)
        {
            m_builder.setInsertBefore(use->getUser());
            auto load = m_builder.emitLoad(globalVar);
            m_builder.replaceOperand(use, load);
        }

        for (auto entryPoint : getEntryPointsWithUsers(getTargetBuiltinInst))
        {
            addTargetBuiltinToEntryPoint(entryPoint, globalVar, getTargetBuiltinInst, builtinType);
        }

        getTargetBuiltinInst->removeAndDeallocate();
    }

    // Returns a set of entry points that has users of the (hoistable) get target builtin
    // instruction.
    HashSet<IRFunc*> getEntryPointsWithUsers(IRGetTargetBuiltin* inst)
    {
        List<IRInst*> users;
        for (auto use = inst->firstUse; use != nullptr; use = use->nextUse)
        {
            auto user = use->getUser();
            SLANG_ASSERT(user);
            users.add(user);
        }

        HashSet<IRFunc*> entryPointsWithUsers;
        for (auto user : users)
        {
            // The entry point reference graph does not track all instructions types.
            // Make sure the user instruction type is included inside the reference graph
            // building logic.
            auto referencingEntryPoints =
                getReferencingEntryPoints(m_entryPointReferenceGraph, user);
            if (!referencingEntryPoints)
                continue;

            for (auto entryPoint : *referencingEntryPoints)
            {
                entryPointsWithUsers.add(entryPoint);
            }
        }

        return entryPointsWithUsers;
    }

    // Adds a new builtin(system value) to the entry point parameter list and assigns the global var
    // to the builtin param.
    void addTargetBuiltinToEntryPoint(
        IRFunc* entryPoint,
        IRGlobalVar* globalVar,
        IRGetTargetBuiltin* getTargetBuiltin,
        IRType* builtinType)
    {
        m_builder.setInsertBefore(entryPoint->getFirstBlock()->getFirstOrdinaryInst());
        auto param = m_builder.emitParam(builtinType);
        m_builder.addTargetSystemValueDecoration(param, getTargetBuiltin->getTargetBuiltinName());
        m_builder.emitStore(globalVar, param);
    }

    List<IRGetTargetBuiltin*> m_getTargetBuiltinInsts;
    Dictionary<IRInst*, HashSet<IRFunc*>> m_entryPointReferenceGraph;
    IRModule* m_module;
    IRBuilder m_builder;
};

void legalizeGetTargetBuiltins(IRModule* module)
{
    LegalizeGetTargetBuiltinContext(module).legalize();
}

} // namespace Slang
