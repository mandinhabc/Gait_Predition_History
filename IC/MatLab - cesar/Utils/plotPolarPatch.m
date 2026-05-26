function plotPolarPatch(varargin)

if nargin>=2
    rr = varargin{1};
    aa = varargin{2};
end
if nargin==3
    arg = varargin{3};
end
if isfield(arg,'Color')
    bgColor = arg.('Color');
else
    bgColor = [1 1 1];
end

[x,y] = plotPolar(linspace(0,2*pi,50),max(rr));
p = patch(x,y,bgColor,'EdgeColor','None');
for r = 2:numel(rr)
    [x,y] = plotPolar(linspace(0,2*pi,50),rr(r));
    lr = plot(x,y,'Color',[1 1 1]*.8,'LineStyle',':');
    lr = get(lr);
    if(r == 2)
        [x1,y1] = plotPolar(aa,rr(r));
        [x2,y2] = plotPolar(aa,max(rr));
        
        for x_idx = 1:numel(x1)
            if(x_idx  > 5 ); break; end
            plot([x1(x_idx) x2(x_idx)],[y1(x_idx) y2(x_idx)],...
                  'Color',lr.Color,'LineStyle',lr.LineStyle);
            if isfield(arg,'labelFormat')
                if ~isequal(arg.('labelFormat'),'')
                    text(x2(x_idx),y2(x_idx),sprintf(arg.('labelFormat'),x_idx),'Interpreter','latex',...
                        'VerticalAlignment','middle','HorizontalAlignment','center',...
                        'FontUnits','points','FontSize',15);
                end
            end
        end

    end
end
