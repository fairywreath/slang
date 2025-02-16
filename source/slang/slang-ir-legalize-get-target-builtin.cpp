#include "slang-ir-legalize-get-target-builtin.h"

#include "slang-ir-call-graph.h"
#include "slang-ir-insts.h"


namespace Slang
{

class LegalizeGetTargetBuiltinContext
{
public:
    explicit LegalizeGetTargetBuiltinContext(IRModule* module)
        : m_builder(module)
    {
        for (auto inst : module->getGlobalInsts())
        {
            if (auto getTargetBuiltinInst = as<IRGetTargetBuiltin>(inst))
            {
                m_getTargetBuiltinInsts.add(getTargetBuiltinInst);
            }
        }

        if (m_getTargetBuiltinInsts.getCount() > 0)
        {
            buildEntryPointReferenceGraph(m_entryPointReferenceGraph, module);
        }
    }

    void legalize()
    {
        for (auto inst : m_getTargetBuiltinInsts)
        {
            legalizeInst(inst);
        }
    }

private:
    void legalizeInst(IRGetTargetBuiltin* getTargetBuiltinInst)
    {
        // The get target builtin instruction is replaced by a global-scope variable.
        auto globalVar = m_builder.createGlobalVar(getTargetBuiltinInst->getDataType());

        HashSet<IRFunc*> entryPointsWithUsers;

        for (auto use = getTargetBuiltinInst->firstUse; use != nullptr; use = use->nextUse)
        {
            m_builder.setInsertBefore(use->getUser());
            auto load = m_builder.emitLoad(globalVar);
            m_builder.replaceOperand(use, load);

            // The entry point reference graph does not track all instructions types.
            // Make sure the user instruction type is included inside the reference graph
            // building logic.
            auto referencingEntryPoints =
                getReferencingEntryPoints(m_entryPointReferenceGraph, use->getUser());
            SLANG_ASSERT(referencingEntryPoints);
            for (auto entryPoint : *referencingEntryPoints)
            {
                entryPointsWithUsers.add(entryPoint);
            }
        }

        for (auto entryPoint : entryPointsWithUsers)
        {
            addTargetBuiltinToEntryPoint(entryPoint, globalVar, getTargetBuiltinInst);
        }

        getTargetBuiltinInst->removeAndDeallocate();
    }

    // Adds a new builtin(system value) to the entry point parameter list and assigns the global var
    // to the builtin param.
    void addTargetBuiltinToEntryPoint(
        IRFunc* entryPoint,
        IRGlobalVar* globalVar,
        IRGetTargetBuiltin* getTargetBuiltin)
    {
        m_builder.setInsertBefore(entryPoint->getFirstBlock()->getFirstOrdinaryInst());
        auto param = m_builder.emitParam(getTargetBuiltin->getDataType());
        m_builder.addTargetSystemValueDecoration(param, getTargetBuiltin->getTargetBuiltinName());
        m_builder.emitStore(globalVar, param);

        auto emptyTypeLayout = IRTypeLayout::Builder(&m_builder).build();
        auto emptyVarLayout = IRVarLayout::Builder(&m_builder, emptyTypeLayout).build();
        m_builder.addLayoutDecoration(param, emptyVarLayout);
    }

    List<IRGetTargetBuiltin*> m_getTargetBuiltinInsts;
    Dictionary<IRInst*, HashSet<IRFunc*>> m_entryPointReferenceGraph;
    IRBuilder m_builder;
};

void legalizeGetTargetBuiltins(IRModule* module)
{
    LegalizeGetTargetBuiltinContext(module).legalize();
}

} // namespace Slang
