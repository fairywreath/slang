#include "slang-ir-call-graph.h"

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

void buildEntryPointReferenceGraph(
    Dictionary<IRInst*, HashSet<IRFunc*>>& referencingEntryPoints,
    IRModule* module,
    Dictionary<IRInst*, HashSet<IRFunc*>>* referencingFunctions,
    Dictionary<IRFunc*, HashSet<IRCall*>>* referencingCalls)
{
    struct WorkItem
    {
        IRFunc* entryPoint;
        IRInst* inst;
        IRFunc* parentFunc;

        HashCode getHashCode() const
        {
            return combineHash(Slang::getHashCode(entryPoint), Slang::getHashCode(inst));
        }
        bool operator==(const WorkItem& other) const
        {
            return entryPoint == other.entryPoint && inst == other.inst;
        }
    };
    HashSet<WorkItem> workListSet;
    List<WorkItem> workList;
    auto addToWorkList = [&](WorkItem item)
    {
        if (workListSet.add(item))
            workList.add(item);
    };

    auto registerEntryPointReference = [&](IRFunc* entryPoint, IRInst* inst)
    {
        if (auto set = referencingEntryPoints.tryGetValue(inst))
            set->add(entryPoint);
        else
        {
            HashSet<IRFunc*> newSet;
            newSet.add(entryPoint);
            referencingEntryPoints.add(inst, _Move(newSet));
        }
    };

    const auto registerFunctionReference = [&](IRFunc* func, IRInst* inst)
    {
        if (referencingFunctions && func)
        {
            if (auto set = referencingFunctions->tryGetValue(inst))
                set->add(func);
            else
            {
                HashSet<IRFunc*> newSet;
                newSet.add(func);
                referencingFunctions->add(inst, _Move(newSet));
            }
        }
    };

    auto registerCallReference = [&](IRCall* call, IRFunc* func)
    {
        if (referencingCalls)
        {
            if (auto set = referencingCalls->tryGetValue(func))
                set->add(call);
            else
            {
                HashSet<IRCall*> newSet;
                newSet.add(call);
                referencingCalls->add(func, _Move(newSet));
            }
        }
    };

    auto visit = [&](IRFunc* entryPoint, IRInst* inst, IRFunc* parentFunc)
    {
        if (auto code = as<IRGlobalValueWithCode>(inst))
        {
            registerEntryPointReference(entryPoint, inst);
            registerFunctionReference(parentFunc, inst);

            if (auto func = as<IRFunc>(code))
            {
                parentFunc = func;
            }

            for (auto child : code->getChildren())
            {
                addToWorkList({entryPoint, child, parentFunc});
            }
            return;
        }
        switch (inst->getOp())
        {
        // Only these instruction types are registered to the entry point reference graph.
        case kIROp_GlobalParam:
        case kIROp_SPIRVAsmOperandBuiltinVar:
        case kIROp_ImplicitSystemValue:
            registerEntryPointReference(entryPoint, inst);
            registerFunctionReference(parentFunc, inst);
            break;

        case kIROp_Block:
        case kIROp_SPIRVAsm:
            for (auto child : inst->getChildren())
            {
                addToWorkList({entryPoint, child, parentFunc});
            }
            break;
        case kIROp_Call:
            registerEntryPointReference(entryPoint, inst);
            registerFunctionReference(parentFunc, inst);
            {
                auto call = as<IRCall>(inst);
                registerCallReference(call, as<IRFunc>(call->getCallee()));
                addToWorkList({entryPoint, call->getCallee(), parentFunc});
            }
            break;
        case kIROp_SPIRVAsmOperandInst:
            {
                auto operand = as<IRSPIRVAsmOperandInst>(inst);
                addToWorkList({entryPoint, operand->getValue(), parentFunc});
            }
            break;
        }
        for (UInt i = 0; i < inst->getOperandCount(); i++)
        {
            auto operand = inst->getOperand(i);
            switch (operand->getOp())
            {
            case kIROp_GlobalParam:
            case kIROp_GlobalVar:
            case kIROp_SPIRVAsmOperandBuiltinVar:
                addToWorkList({entryPoint, operand, parentFunc});
                break;
            }
        }
    };

    for (auto globalInst : module->getGlobalInsts())
    {
        if (globalInst->getOp() == kIROp_Func &&
            globalInst->findDecoration<IREntryPointDecoration>())
        {
            auto entryPointFunc = as<IRFunc>(globalInst);
            visit(entryPointFunc, globalInst, nullptr);
        }
    }
    for (Index i = 0; i < workList.getCount(); i++)
        visit(workList[i].entryPoint, workList[i].inst, workList[i].parentFunc);
}

HashSet<IRFunc*>* getReferencingEntryPoints(
    Dictionary<IRInst*, HashSet<IRFunc*>>& m_referencingEntryPoints,
    IRInst* inst)
{
    auto* referencingEntryPoints = m_referencingEntryPoints.tryGetValue(inst);
    if (!referencingEntryPoints)
        return nullptr;
    return referencingEntryPoints;
}

} // namespace Slang
