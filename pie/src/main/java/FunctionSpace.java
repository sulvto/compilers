/**
 * Created by sulvto on 17-10-27.
 */
public class FunctionSpace extends MemorySpace {
    FunctionSymbol def;

    public FunctionSpace(FunctionSymbol func) {
        super(func.name + " invocation");
        this.def = func;
    }
}
