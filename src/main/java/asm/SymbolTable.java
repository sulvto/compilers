package asm;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by sulvto on 17-11-4.
 */
public class SymbolTable {
    private String base;
    private Map<UnnamedSymbol, String> map;
    private long seq = 0;

    static private final String DUMMY_SYMBOL_BASE = "L";
    static private final SymbolTable dummy = new SymbolTable(DUMMY_SYMBOL_BASE);

    static public SymbolTable dummy() {
        return dummy;
    }

    public SymbolTable(String base) {
        this.base = base;
        this.map = new HashMap<>();
    }

    public Symbol newSymbol() {
        return new NamedSymbol(newString());
    }

    public String symbolString(UnnamedSymbol symbol) {
        String str = map.get(symbol);
        if (str != null) {
            return str;
        } else {
            String newStr = newString();
            map.put(symbol, newStr);
            return newStr;
        }
    }

    private String newString() {
        return base + seq++;
    }
}
