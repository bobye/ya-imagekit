function [ idx, d ] = gettheme( depth, d0, V, V0, varargin)


maxiter = 50;
idx0 = [];
idx = [];
accu = Inf;



if (nargin >= 5)
    idx0 = varargin{1};
    for i=1:length(idx0)
        d0 = d0.* pairreward(idx0(i));
    end
end

d=d0;

while accu>maxiter
    accu = 0;
    idx = idx0;
    d = getthemeiter(length(idx)+1, d0);
end

    function [d1] = getthemeiter(iter, d0)        
        thres = 0.7;
        d1= d0;
        accu = accu +1;
        if accu > maxiter            
        elseif iter > depth
        elseif all(d0<thres)
            idx(end) = [];
            d1 = d0;            
        else       
            d1 = d0;
            while (length(idx) < iter)
                idx(iter) = randsample(find(d0>=thres),1);
                d1 = getthemeiter(iter+1, d0.*pairreward(idx(iter)));
            end             
        end        
    end


function d = pairreward( idx )

sv = V(idx,:);
d = repmat(sv, [size(V,1) 1]) - V; 
d = sqrt(sum(d.^2, 2));
d = exp( - d.^2/mean(d)^2);

sv0 = V0(idx,:);
d0 = repmat(sv0, [size(V0,1) 1]) - V0; 
d0 = sqrt(sum(d0.^2, 2));
d0 = exp( - d0.^2/mean(d0)^2);

d= d./d0; %d(isnan(d)) = 1;
d = exp( - d.^2/mean(d)^2);

%hist(d,30);

end

end

