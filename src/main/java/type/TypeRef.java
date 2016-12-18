package type;

import ast.Location;

/**
 * Created by sulvto on 16-12-18.
 */
public abstract class TypeRef {
    private Location location;

    public TypeRef(Location location) {
        this.location = location;
    }

    public Location location() {
        return location;
    }
}
