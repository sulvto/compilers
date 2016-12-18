package type;

import ast.Location;

/**
 * Created by sulvto on 16-12-18.
 */
public class VoidTypeRef extends TypeRef {
    public VoidTypeRef(Location location) {
        super(location);
    }

    public VoidTypeRef() {
        super(null);
    }

    @Override
    public String toString() {
        return "void";
    }
}
