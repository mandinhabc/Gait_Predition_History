function [varargout] = plotPolar(varargin)

    switch (nargin)
        case 1
            X = varargin{1};
        case 2
            X = varargin{1};
            Y = varargin{2};
    end

    XX = Y.*cos(X);
    YY = Y.*sin(X);
    
    p = 0;
    if(nargout == 0 || nargout == 1 || nargout == 3)
        axis equal
        p = plot(XX,YY);
    end
    
    
    switch(nargout)
        case 1
            varargout{1} = p;
        case 2
            varargout{1} = XX;
            varargout{2} = YY;
        case 3
            varargout{1} = XX;
            varargout{2} = YY;
            varargout{3} = p;
    end
    
    
    
