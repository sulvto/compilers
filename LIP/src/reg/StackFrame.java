package reg;

import assembler.FunctionSymbol;

/**
 * Created by sulvto on 17-10-30.
 */
public class StackFrame {
    public final FunctionSymbol fun;
    public final int returnAddress;
    public final Object[] register;

    public StackFrame(FunctionSymbol fun, int returnAddress) {
        this.fun = fun;
        this.returnAddress = returnAddress;
        register = new Object[fun.getNargs() + fun.getNlocals() + 1];
    }
}
