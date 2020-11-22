p = 5; %Warianty palet
r = 9; %Warianty skalowania
t = 3; %Prób przy random
i = 64; %Iloœæ obrazów

time_per_img = 1;

non_random_palette  = p*(3 + 2);
random_palette      = p*(1 + 2);
non_random_rescale  = r*3;
random_rescale      = r*2;

a = non_random_palette * non_random_rescale;
b = non_random_palette * random_rescale * t;
c = random_palette * non_random_rescale * t;
d = random_palette * random_rescale * t;
e = non_random_palette + random_palette;

x = (a + b + c + d + e) * i

(x * time_per_img) / 60 /60


