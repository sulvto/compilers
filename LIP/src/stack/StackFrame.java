package stack;

import assembler.FunctionSymbol;

/**
 * Created by sulvto on 17-10-29.
 */
public class StackFrame {
    public final FunctionSymbol fun;
    public final int returnAddress;
    public final Object[] locals;

    public StackFrame(FunctionSymbol fun, int returnAddress) {
        this.fun = fun;
        this.returnAddress = returnAddress;
        locals = new Object[fun.getNargs() + fun.getNlocals()];
    }
}
