function D = sqrtDistance(p1, p2)
    L1 = evaluate(p1, p2);
    L2 = evaluate(p2, p2);
    W = getWeights(p2);
    D = (sqrt(L1).*sqrt(L2) * W') / (L2 * W');
end