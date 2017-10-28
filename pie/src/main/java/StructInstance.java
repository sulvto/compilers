/**
 * Created by sulvto on 17-10-27.
 */
public class StructInstance extends MemorySpace {
    StructSymbol def;

    public StructInstance(StructSymbol struct) {
        super(struct.name + " instance");
        this.def = struct;
    }

    @Override
    public String toString() {
        return "{" + def.fields.entrySet().stream().map(stringSymbolEntry -> stringSymbolEntry.getKey() + "=" + stringSymbolEntry.getValue()).reduce((s, s2) -> s + "," + s2).orElse("") + "}";
    }
}
