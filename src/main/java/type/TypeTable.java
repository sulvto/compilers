package type;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by sulvto on 17-11-2.
 */
public class TypeTable {

    private Map<TypeRef, Type> table;

    public TypeTable() {
        this.table = new HashMap<>();
    }

    public boolean isDefined(TypeRef ref) {
        return table.containsKey(ref);
    }

    public void put(TypeRef ref, Type type) {
        if (table.containsKey(ref)) {
            throw new Error("duplicated type definition:" + ref);
        }
        table.put(ref, type);
    }
}
