package type;

import ast.Location;

/**
 * Created by sulvto on 16-12-18.
 */
public class StructTypeRef extends TypeRef {
    private String name;

    public StructTypeRef(String name) {
        super(null);
        this.name = name;
    }

    public StructTypeRef(Location location, String name) {
        super(location);
        this.name = name;
    }

    @Override
    public String toString() {
        return "struct " + name;
    }
}
