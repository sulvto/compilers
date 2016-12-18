package type;

import ast.Location;

/**
 * Created by sulvto on 16-12-18.
 */
public class PointerTypeRef extends TypeRef {
    private TypeRef baseRef;

    public PointerTypeRef(TypeRef baseRef) {
        super(baseRef.location());
        this.baseRef = baseRef;
    }

    @Override
    public String toString() {
        return baseRef.toString() + "*";
    }
}
