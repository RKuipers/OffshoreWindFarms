def calc(minutes: int) -> (int, int):
    worku = 86
    breaku = 40

    res = divmod((minutes-worku), (worku + breaku))

    return (res[0] + 1, res[1])

