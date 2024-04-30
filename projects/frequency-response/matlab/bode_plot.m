%%

% the data is found in an m-file named "sweep"
frequency       = x(:,1);
analogAmplitude = x(:,2);
analogPhase     = x(:,3);
audioAmplitude  = x(:,4);
audioPhase      = x(:,5);

fmin = min(frequency(frequency>0));
fmax = max(frequency);

close all
figure(3)
set(gcf, 'Position', [50 50 480 400])

subplot(2,1,1)
loglog(frequency, analogAmplitude, 'b.-'); title('analogAmplitude')
axis([fmin fmax 0.1 10]);

subplot(2,1,2)
semilogx(frequency, analogPhase, 'b.-'); title('analogPhase')
axis([fmin fmax -180 180]);

%%

close all
figure(4)
set(gcf, 'Position', [50 50 480 400])

subplot(2,1,1)
loglog(frequency, audioAmplitude, 'g.-'); title('audioAmplitude')
axis([fmin fmax 0.1 10]);

subplot(2,1,2)
semilogx(frequency, audioPhase, 'g.-'); title('audioPhase')
axis([fmin fmax -180 180]);

%%

figure
plot(frequency, unwrap(analogPhase), 'b.-');
hold on
plot(frequency, unwrap(audioPhase), 'g.-');

% p = polyfit(frequency, unwrap(analogPhase), 1)
% p = polyfit(frequency, unwrap(audioPhase), 1)
