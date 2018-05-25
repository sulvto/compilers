import java.util.LinkedHashMap;
import java.util.Map;

/**
 * Created by sulvto on 17-10-27.
 */
public class FunctionSymbol extends ScopedSymbol {
    public PieAST blockAST;
    Map<String, Symbol> formalArgs = new LinkedHashMap<>();

    public FunctionSymbol(String name, Scope parent) {
        super(name, parent);
    }

    @Override
    public Map<String, Symbol> getMembers() {
        return formalArgs;
    }

    @Override
    public String getName() {
        return name + "(" + formalArgs.keySet().toString() + ")";
    }
}
