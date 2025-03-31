@generic OpenBox.#T (@type T)
    @struct {
        T contents
    }

@generic ClosedBox.#T (@type T)
    @struct {
        T _contents
    }

    @inline new(T contents =>#self)
        ClosedBox { _contents: contents }
